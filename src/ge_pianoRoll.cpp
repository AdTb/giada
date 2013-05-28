/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_pianoRoll
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


#include <FL/fl_draw.H>
#include "ge_pianoRoll.h"
#include "gd_mainWindow.h"
#include "gd_actionEditor.h"
#include "channel.h"
#include "const.h"


extern gdMainWindow *mainWin;


gPianoRollContainer::gPianoRollContainer(int x, int y, class gdActionEditor *pParent)
 : Fl_Scroll(x, y, 200, 80), pParent(pParent)
{
	size(pParent->totalWidth, h());
	type(0); // no scrollbars

	pianoRoll = new gPianoRoll(x, y, pParent->totalWidth, pParent);
}


/* ------------------------------------------------------------------ */


void gPianoRollContainer::draw() {

	pianoRoll->size(this->w(), pianoRoll->h());  /// <--- not optimal


	/* clear background */

	fl_rectf(x(), y(), w(), h(), COLOR_BG_MAIN);

	fl_push_clip(x(), y(), w(), h());
	draw_child(*pianoRoll);
	fl_pop_clip();

	fl_color(COLOR_BD_0);
	fl_line_style(0);
	fl_rect(x(), y(), pParent->totalWidth, h());
}


/* ------------------------------------------------------------------ */

#if 0
int gPianoRollContainer::handle(int e) {

	int ret = Fl_Group::handle(e);

	switch (e) {
		case FL_PUSH:	{
			ret = 1;
			break;
		}
		case FL_DRAG: {
			if (Fl::event_button3())
				scroll_to(xposition(), y()-Fl::event_y());
			ret = 1;
			break;
		}
	}

	return ret;
}
#endif


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gPianoRoll::gPianoRoll(int x, int y, int w, class gdActionEditor *pParent)
 : gActionWidget(x, y, w, 40, pParent)
{
	size(w, 128 * 15);  // 128 MIDI channels * 15 px height
	drawSurface();
}


/* ------------------------------------------------------------------ */


void gPianoRoll::drawSurface() {
	surface = fl_create_offscreen(w(), h());
	fl_begin_offscreen(surface);

	/* warning: only w() and h() come from this widget, x and y coordinates
	 * are absolute, since we are writing in a memory chunk */

	fl_rectf(0, 0, w(), h(), COLOR_BG_MAIN);

	fl_color(fl_rgb_color(54, 54, 54));
	fl_line_style(FL_DASH, 0, NULL);
	fl_font(FL_HELVETICA, 11);

	int octave = 9;

	for (int i=1; i<=128; i++) {

		/* print key note label. C C# D D# E F F# G G# A A# B */

		char note[6];
		int  step = i % 12;

		if      (step == 1)
			sprintf(note, "%dG", octave);
		else if (step == 2)
			sprintf(note, "%dF#", octave);
		else if (step == 3)
			sprintf(note, "%dF", octave);
		else if (step == 4)
			sprintf(note, "%dE", octave);
		else if (step == 5)
			sprintf(note, "%dD#", octave);
		else if (step == 6)
			sprintf(note, "%dD", octave);
		else if (step == 7)
			sprintf(note, "%dC#", octave);
		else if (step == 8)
			sprintf(note, "%dC", octave);
		else if (step == 9)
			sprintf(note, "%dB", octave);
		else if (step == 10)
			sprintf(note, "%dA#", octave);
		else if (step == 11)
			sprintf(note, "%dA", octave);
		else if (step == 0) {
			sprintf(note, "%dG#", octave);
			octave--;
		}

		fl_draw(note, 4, ((i-1)*15)+1, 30, 15, (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CENTER));

		/* print horizontal line */

		if (i < 128)
			fl_line(0, i*15, x()+w()-2, +i*15);
	}
	fl_line_style(0);
	fl_end_offscreen();
}


/* ------------------------------------------------------------------ */


void gPianoRoll::draw() {
	fl_copy_offscreen(x(), y(), w(), h(), surface, 0, 0);
	baseDraw(false);
	draw_children();
}


/* ------------------------------------------------------------------ */


int gPianoRoll::handle(int e) {

	int ret = Fl_Group::handle(e);

	switch (e) {
		case FL_PUSH:	{

			push_y = Fl::event_y() - y();

			if (Fl::event_button1()) {

				/* ax is driven by grid, ay by the height in px of each note */

				int ax = Fl::event_x();
				int ay = Fl::event_y();

				/* vertical snap */

				int edge = (ay-y()-3) % 15;
				if (edge != 0) ay -= edge;

				/* horizontal snap (grid tool) TODO */

				if (!onItem()) {
					add(new gPianoItem(ax, ay, ax-x(), ay-y()-3, NULL, pParent));
					redraw();
				}
			}
			ret = 1;
			break;
		}
		case FL_DRAG:	{

			if (Fl::event_button3()) {

				gPianoRollContainer *prc = (gPianoRollContainer*) parent();
				position(x(), Fl::event_y() - push_y);

				if (y() > prc->y())
					position(x(), prc->y());
				else
				if (y() < prc->y()+prc->h()-h())
					position(x(), prc->y()+prc->h()-h());

				prc->redraw();
			}
			ret = 1;
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


bool gPianoRoll::onItem() {
	int n = children();
	for (int i=0; i<n; i++) {   // no scrollbars to skip
		gPianoItem *p = (gPianoItem*) child(i);
		if (Fl::event_x() >= p->x()          &&
		    Fl::event_x() <= p->x() + p->w() &&
		    Fl::event_y() >= p->y()          &&
		    Fl::event_y() <= p->y() + p->h())
		{
			return true;
		}

	}
	return false;
}

/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gPianoItem::gPianoItem(int x, int y, int rel_x, int rel_y, recorder::action *a, gdActionEditor *pParent)
	: Fl_Box  (x, y, 20, 10),
	  a       (a),
		pParent (pParent),
		selected(false),
		event_a (0x00),
		event_b (0x00),
		changed (false)
{
	if (a)
		puts("[gPianoItem] new gPianoItem, display mode");
	else {
		note    = getNote(rel_y);
		frame_a = rel_x * pParent->zoom;
		frame_b = (rel_x + 20) * pParent->zoom;

		/** TODO - resize this according to pParent->zoom and frame_a+frame_b*/

		//printf("[gPianoItem] new gPianoItem, record mode, note %d, frame_a %d, frame_b %d\n", note, frame_a, frame_b);
		record();
		mainWin->keyboard->setChannelWithActions(pParent->chan); // mainWindow update
	}
}


/* ------------------------------------------------------------------ */


void gPianoItem::draw() {
	fl_rectf(x(), y(), w(), h(), (Fl_Color) selected ? COLOR_BD_1 : COLOR_BG_2);
}


/* ------------------------------------------------------------------ */


void gPianoItem::record() {

	/* note on */

	event_a |= (0x90 << 24);   // note on
	event_a |= (note << 16);   // note value
	event_a |= (0x3F <<  8);   // velocity
	event_a |= (0x00);
	recorder::rec(pParent->chan->index, ACTION_MIDI, frame_a, event_a);

	/* note off */

	event_b |= (0x80 << 24);   // note off
	event_b |= (note << 16);   // note value
	event_b |= (0x3F <<  8);   // velocity
	event_b |= (0x00);
	recorder::rec(pParent->chan->index, ACTION_MIDI, frame_b, event_b);
}


/* ------------------------------------------------------------------ */


void gPianoItem::remove() {
	recorder::deleteAction(pParent->chan->index, frame_a, ACTION_MIDI, true, event_a, 0.0);
	recorder::deleteAction(pParent->chan->index, frame_b, ACTION_MIDI, true, event_b, 0.0);
}


/* ------------------------------------------------------------------ */


int gPianoItem::handle(int e) {

	int ret = 0;

	switch (e) {

		case FL_ENTER: {
			selected = true;
			ret = 1;
			redraw();
			break;
		}

		case FL_LEAVE: {
			fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
			selected = false;
			ret = 1;
			redraw();
			break;
		}

		case FL_MOVE: {
			onLeftEdge  = false;
			onRightEdge = false;

			if (Fl::event_x() >= x() && Fl::event_x() < x()+4) {
				onLeftEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
			if (Fl::event_x() >= x()+w()-4 && Fl::event_x() <= x()+w()) {
				onRightEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
				fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);

			ret = 1;
			break;
		}

		case FL_PUSH: {

			push_x = Fl::event_x() - x();

			if (Fl::event_button3()) {
				fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
				remove();
				Fl::delete_widget(this);
				mainWin->keyboard->setChannelWithActions(pParent->chan);  // update mainwin
				((gPianoRoll*)parent())->redraw();
			}
			ret = 1;
			break;
		}

		case FL_DRAG: {

			changed = true;
			gPianoRoll *pr = (gPianoRoll*) parent();
			int nx, ny, nw;

			if (onLeftEdge) {
				nx = Fl::event_x();
				ny = y();
				nw = x()-Fl::event_x()+w();
				if (nx < pr->x()) {
					nx = pr->x();
					nw = w()+x()-pr->x();
				}
				else
				if (nx > x()+w()-8) {
					nx = x()+w()-8;
					nw = 8;
				}
				resize(nx, ny, nw, h());
			}
			else
			if (onRightEdge) {
				nw = Fl::event_x()-x();
				if (Fl::event_x() < x()+8)
					nw = 8;
				else
				if (Fl::event_x() > pParent->coverX)
					nw = pParent->coverX-x();
				size(nw, h());
			}
			else {
				nx = Fl::event_x() - push_x;
				if (nx < pr->x())
					nx = pr->x();
				else
				if (nx+w() > pParent->coverX)
					nx = pParent->coverX-w();
				position(nx, y());
			}

			/* update screen */

			redraw();
			((gPianoRoll*)parent())->redraw();
			ret = 1;
			break;
		}

		case FL_RELEASE: {

			/* delete & record the action */

			if (changed) {
				remove();
				note    = getNote(getRelY());
				frame_a = getRelX() * pParent->zoom;
				frame_b = (getRelX()+w()) * pParent->zoom;
				record();

				changed = false;
			}
			ret = 1;
			break;
		}
	}
	return ret;
}
