/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_actionEditor
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#include <math.h>
#include "gd_actionEditor.h"
#include "ge_actionChannel.h"
#include "ge_muteChannel.h"
#include "ge_envelopeChannel.h"
#include "gui_utils.h"
#include "mixer.h"
#include "recorder.h"
#include "conf.h"
#include "ge_mixed.h"
#include "channel.h"


extern Mixer G_Mixer;
extern Conf	 G_Conf;


gdActionEditor::gdActionEditor(channel *chan)
: gWindow(640, 284), chan(chan), zoom(100)
{

	if (G_Conf.actionEditorW) {
		resize(G_Conf.actionEditorX, G_Conf.actionEditorY, G_Conf.actionEditorW, G_Conf.actionEditorH);
		zoom = G_Conf.actionEditorZoom;
	}

	/* compute values */

	calc();

	/* container with zoom buttons and the action type selector. Scheme of
	 * the resizable boxes: |[--b1--][actionType][--b2--][+][-]| */

	Fl_Group *upperArea = new Fl_Group(8, 8, w()-16, 20);
	upperArea->begin();
	  actionType = new gChoice(8, 8, 80, 20);
	  gridTool   = new gGridTool(actionType->x()+actionType->w()+4, 8, this);
		gBox *b1   = new gBox(gridTool->x()+gridTool->w()+4, 8, 300, 20);    // padding actionType - zoomButtons
		zoomIn     = new gClick(w()-8-40-4, 8, 20, 20, "+");
		zoomOut    = new gClick(w()-8-20,   8, 20, 20, "-");
	upperArea->end();
	upperArea->resizable(b1);

	actionType->add("key press");
	actionType->add("key release");
	actionType->add("kill chan");
	actionType->value(0);

	gridTool->init(G_Conf.actionEditorGridVal, G_Conf.actionEditorGridOn);
	gridTool->calc();

	if (chan->mode == SINGLE_PRESS ||
			chan->mode & LOOP_ANY)
		actionType->deactivate();

	zoomIn->callback(cb_zoomIn, (void*)this);
	zoomOut->callback(cb_zoomOut, (void*)this);

	/* main scroller: contains all widgets */

	scroller = new Fl_Scroll(8, 36, this->w()-16, this->h()-44);
	scroller->type(Fl_Scroll::HORIZONTAL);
	scroller->hscrollbar.color(COLOR_BG_0);
	scroller->hscrollbar.selection_color(COLOR_BG_1);
	scroller->hscrollbar.labelcolor(COLOR_BD_1);
	scroller->hscrollbar.slider(G_BOX);

	ac = new gActionChannel  (scroller->x(), upperArea->y()+upperArea->h()+8, this);
	mc = new gMuteChannel    (scroller->x(), ac->y()+ac->h()+8, this);
	vc = new gEnvelopeChannel(scroller->x(), mc->y()+mc->h()+8, this, ACTION_VOLUME, RANGE_FLOAT, "volume");
	scroller->add(ac);
	scroller->add(new gResizerBar(ac->x(), ac->y()+ac->h(), scroller->w(), 8));
	scroller->add(mc);
	scroller->add(new gResizerBar(mc->x(), mc->y()+mc->h(), scroller->w(), 8));
	scroller->add(vc);
	scroller->add(new gResizerBar(vc->x(), vc->y()+vc->h(), scroller->w(), 8));

	end();

	/* fill volume envelope with actions from recorder */

	vc->fill();

	/* if channel is LOOP_ANY, deactivate it: a loop mode channel cannot
	 * hold keypress/keyrelease actions */

	if (chan->mode & LOOP_ANY)
		ac->deactivate();

	gu_setFavicon(this);

	char buf[256];
	sprintf(buf, "Edit Actions in Channel %d", chan->index+1);
	label(buf);

	set_non_modal();
	size_range(640, 284);
	resizable(scroller);

	show();
}


/* ------------------------------------------------------------------ */


gdActionEditor::~gdActionEditor() {
	G_Conf.actionEditorX = x();
	G_Conf.actionEditorY = y();
	G_Conf.actionEditorW = w();
	G_Conf.actionEditorH = h();
	G_Conf.actionEditorZoom = zoom;
}


/* ------------------------------------------------------------------ */


void gdActionEditor::cb_zoomIn(Fl_Widget *w, void *p)  { ((gdActionEditor*)p)->__cb_zoomIn(); }
void gdActionEditor::cb_zoomOut(Fl_Widget *w, void *p) { ((gdActionEditor*)p)->__cb_zoomOut(); }


/* ------------------------------------------------------------------ */


void gdActionEditor::__cb_zoomIn() {
	if (zoom <= 8)
		return;
	zoom /= 2;
	totalWidth = (int) ceilf(totalFrames / (float) zoom);

	/* FLTK 1.3.x doesn't seem to support widget width > 16 bit signed
	 * (32767 max) */

	if (totalWidth > 32767) {
		totalWidth = 32760;
		zoom = (int) ceilf(totalFrames / (float) totalWidth);
	}

	ac->updateActions();
	mc->updatePoints();
	vc->updatePoints();
	gridTool->calc();
	scroller->redraw();
}


/* ------------------------------------------------------------------ */


void gdActionEditor::__cb_zoomOut() {
	zoom *= 2;
	totalWidth = (int) ceilf(totalFrames / (float) zoom);

	if (totalWidth < scroller->w()) {
		totalWidth = scroller->w();
		zoom = (int) ceilf(totalFrames / (float) totalWidth);
	}

	ac->updateActions();
	mc->updatePoints();
	vc->updatePoints();
	gridTool->calc();
	scroller->redraw();
}


/* ------------------------------------------------------------------ */


void gdActionEditor::calc() {
	framesPerBar   = G_Mixer.framesPerBar;      // we do care about stereo infos, no /2 division
	framesPerBeat  = G_Mixer.framesPerBeat;
	framesPerBeats = framesPerBeat * G_Mixer.beats;
	totalFrames    = framesPerBeat * MAX_BEATS;
	beatWidth      = framesPerBeat / zoom;
	totalWidth     = (int) ceilf(totalFrames / (float) zoom);
}


/* ------------------------------------------------------------------ */


int gdActionEditor::handle(int e) {
	int ret = Fl_Group::handle(e);
	switch (e) {
		case FL_MOUSEWHEEL: {
			if (Fl::event_dy() == -1) __cb_zoomIn();
			else __cb_zoomOut();
			ret = 1;
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


int gdActionEditor::getActionType() {
	if (actionType->value() == 0)
		return ACTION_KEYPRESS;
	else
	if (actionType->value() == 1)
		return ACTION_KEYREL;
	else
	if (actionType->value() == 2)
		return ACTION_KILLCHAN;
	else
		return -1;
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gGridTool::gGridTool(int x, int y, gdActionEditor *parent)
	:	Fl_Group(x, y, 80, 20), parent(parent)
{
	gridType = new gChoice(x, y, 40, 20);
	gridType->add("1");
	gridType->add("2");
	gridType->add("3");
	gridType->add("4");
	gridType->add("6");
	gridType->add("8");
	gridType->add("16");
	gridType->add("32");
	gridType->value(0);
	gridType->callback(cb_changeType, (void*)this);

	active = new gCheck (x+44, y+4, 12, 12);
	end();
}


/* ------------------------------------------------------------------ */


gGridTool::~gGridTool() {
	G_Conf.actionEditorGridVal = gridType->value();
	G_Conf.actionEditorGridOn  = active->value();
}


/* ------------------------------------------------------------------ */


void gGridTool::cb_changeType(Fl_Widget *w, void *p)  { ((gGridTool*)p)->__cb_changeType(); }


/* ------------------------------------------------------------------ */


void gGridTool::__cb_changeType() {
	calc();
	parent->redraw();
}


/* ------------------------------------------------------------------ */


bool gGridTool::isOn() {
	return active->value();
}


/* ------------------------------------------------------------------ */


void gGridTool::init(int v, bool b) {
	gridType->value(v);
	active->value(b);
}


/* ------------------------------------------------------------------ */


int gGridTool::getValue() {
	switch (gridType->value()) {
		case 0:	return 1;
		case 1: return 2;
		case 2: return 3;
		case 3: return 4;
		case 4: return 6;
		case 5: return 8;
		case 6: return 16;
		case 7: return 32;
	}
	return 0;
}


/* ------------------------------------------------------------------ */


void gGridTool::calc() {

	points.clear();
	frames.clear();

	/* same algorithm used in ge_actionWidget::draw() and wave display */

	bool end = false;
	int  j   = 0;
	int fpgc = floor(parent->framesPerBeat / getValue());  // frames per grid cell

	for (int i=1; i<parent->totalWidth && !end; i++) {   // if i=0, step=0 -> useless cycle
		int step = parent->zoom*i;
		while (j < step && j < parent->framesPerBeats) {
			if (j % fpgc == 0) {
				//printf("   grid frame found at %d, pixel %d\n", j, i);
				points.add(i);
				frames.add(j);
			}
			j++;
		}
		j = step;
	}
}


/* ------------------------------------------------------------------ */


int gGridTool::getSnapPoint(int v) {
	for (int i=0; i<(int)points.size; i++) {

		if (i == (int) points.size-1)
			return points.at(i);

		int gp  = points.at(i);
		int gpn = points.at(i+1);

		if (v >= gp && v < gpn) {
			if (v < gpn) {
				return gp;
				break;
			}
		}
	}
	return v;  // default value
}


/* ------------------------------------------------------------------ */


int gGridTool::getSnapFrame(int v) {

	v *= parent->zoom;  // transformation pixel -> frame

	for (int i=0; i<(int)frames.size; i++) {

		if (i == (int) frames.size-1)
			return frames.at(i);

		int gf  = frames.at(i);     // grid frame
		int gfn = frames.at(i+1);   // grid frame next

		if (v >= gf && v < gfn) {

			/* which one is the closest? gf < v < gfn */

			if ((gfn - v) < (v - gf))
				return gfn;
			else
				return gf;
		}
	}
	return v;  // default value
}


/* ------------------------------------------------------------------ */


int gGridTool::getCellSize() {
	return (parent->coverX - parent->ac->x()) / G_Mixer.beats / getValue();
}
