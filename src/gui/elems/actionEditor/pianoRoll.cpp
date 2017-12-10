/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
 * -------------------------------------------------------------------------- */


#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../../core/mixer.h"
#include "../../../core/clock.h"
#include "../../../core/channel.h"
#include "../../../core/recorder.h"
#include "../../../core/kernelMidi.h"
#include "../../../utils/log.h"
#include "../../../utils/string.h"
#include "../../dialogs/gd_actionEditor.h"
#include "pianoItem.h"
#include "pianoItemOrphaned.h"
#include "noteEditor.h"
#include "pianoRoll.h"


using std::string;
using namespace giada::m;


gePianoRoll::gePianoRoll(int X, int Y, int W, gdActionEditor* pParent)
	: geBaseActionEditor(X, Y, W, 40, pParent)
{
	resizable(nullptr);                   // don't resize children (i.e. pianoItem)
	size(W, (MAX_KEYS+1) * CELL_H);      // 128 MIDI channels * CELL_H height

	if (conf::pianoRollY == -1)
		position(x(), y()-(h()/2));  // center
	else
		position(x(), conf::pianoRollY);

	drawSurface1();
	drawSurface2();

	/* Add actions when the window is opened. Position is zoom-based. MIDI actions
	come always in pair: noteOn + noteOff. */

	recorder::sortActions();

	recorder::action* a2   = nullptr;
	recorder::action* prev = nullptr;

	for (unsigned i=0; i<recorder::frames.size(); i++) {

		if (recorder::frames.at(i) > clock::getTotalFrames()) // don't show actions > gray area
			continue;

		for (unsigned j=0; j<recorder::global.at(i).size(); j++) {

			recorder::action* a1 = recorder::global.at(i).at(j);

			/* Skip action if:
				- does not belong to this channel
				- is not a MIDI action (we only want MIDI things here)
				- is the previous one (we have already checked it)
				- (later on) is not a MIDI Note On type. We don't want any other kind of
					action here */

			if (a1->chan != pParent->chan->index)
				continue;
			if (a1->type != G_ACTION_MIDI)
				continue;
			if (a1 == prev)
				continue;

			/* Extract MIDI infos from a1: if is note off skip it, we are looking for
			noteOn only. */

			int a1_type = kernelMidi::getB1(a1->iValue);
			int a1_note = kernelMidi::getB2(a1->iValue);

			/* If two same notes are found (noteOn-noteOn, noteOff-noteOff) or the
			very first action is a noteOff, add orphaned item.*/

			if ((prev && a1_type == prev->type) || a1_type == 0x80) {
				gu_log("[geNoteEditor] invalid note pair! Add orphaned item\n");
				new gePianoItemOrphaned(0, 0, x(), y(), a1, pParent);
				a2 = nullptr;
				continue;
			}

			/* Now skip anything that is not a noteOn. */

			if (a1_type != 0x90)
				continue;

			/* Search for the next action. Must have: same channel, G_ACTION_MIDI,
			greater than a1->frame and with MIDI properties of note_off (0x80), same
			note of a1, any velocity value (0xFF) because we just don't care about the
			velocity of a note_off. */

			recorder::getNextAction(a1->chan, G_ACTION_MIDI, a1->frame, &a2,
					kernelMidi::getIValue(0x80, a1_note, 0xFF));

			/* Next action note_off found: add a new gePianoItem to piano roll. Add
			an orphaned piano item otherwise.  */

			if (a2) {
				new gePianoItem(0, 0, x(), y(), a1, a2, pParent);
				prev = a2;
				a2 = nullptr;
			}
			else {
				gu_log("[geNoteEditor] noteOff not found! Add orphaned item\n");
				new gePianoItemOrphaned(0, 0, x(), y(), a1, pParent);
			}
		}
	}

	end();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface1()
{
	surface1 = fl_create_offscreen(CELL_W, h());
	fl_begin_offscreen(surface1);

	/* warning: only w() and h() come from this widget, x and y coordinates
	 * are absolute, since we are writing in a memory chunk */

	fl_rectf(0, 0, CELL_W, h(), G_COLOR_GREY_1);

	fl_line_style(FL_DASH, 0, nullptr);
	fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);

	int octave = MAX_OCTAVES;

	for (int i=1; i<=MAX_KEYS+1; i++) {

		/* print key note label. C C# D D# E F F# G G# A A# B */

		string note = gu_iToString(octave);
		switch (i % KEYS) {
			case (int) Notes::G:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " G"; 
				break;
			case (int) Notes::FS:
				note += " F#";
				break;
			case (int) Notes::F:
				note += " F";
				break;
			case (int) Notes::E:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " E";
				break;
			case (int) Notes::DS:
				note += " D#";
				break;
			case (int) Notes::D:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " D";
				break;
			case (int) Notes::CS:
				note += " C#";
				break;
			case (int) Notes::C:
				note += " C";
				octave--;
				break;
			case (int) Notes::B:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " B";
				break;
			case (int) Notes::AS:
				note += " A#";
				break;
			case (int) Notes::A:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " A";
				break;
			case (int) Notes::GS:
				note += " G#";
				break;
		}

		/* Print note name */

		fl_color(G_COLOR_GREY_3);
		fl_draw(note.c_str(), 4, ((i-1)*CELL_H)+1, CELL_W, CELL_H,
			(Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CENTER));

		/* Print horizontal line */

		if (i < MAX_KEYS+1)
			fl_line(0, i*CELL_H, CELL_W, +i*CELL_H);
	}

	fl_line_style(0);
	fl_end_offscreen();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface2()
{
	surface2 = fl_create_offscreen(CELL_W, h());
	fl_begin_offscreen(surface2);
	fl_rectf(0, 0, CELL_W, h(), G_COLOR_GREY_1);
	fl_color(G_COLOR_GREY_3);
	fl_line_style(FL_DASH, 0, nullptr);
	for (int i=1; i<=MAX_KEYS+1; i++) {
		switch (i % KEYS) {
			case (int) Notes::G:
			case (int) Notes::E:
			case (int) Notes::D:
			case (int) Notes::B:
			case (int) Notes::A:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				break;
		}
		if (i < MAX_KEYS+1) {
			fl_color(G_COLOR_GREY_3);
			fl_line(0, i*CELL_H, CELL_W, i*CELL_H);
		}
	}
	fl_line_style(0);
	fl_end_offscreen();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::draw()
{
	fl_copy_offscreen(x(), y(), CELL_W, h(), surface1, 0, 0);

#if defined(__APPLE__)
	for (int i=36; i<pParent->totalWidth; i+=36) /// TODO: i < pParent->coverX is faster
		fl_copy_offscreen(x()+i, y(), CELL_W, h(), surface2, 1, 0);
#else
	for (int i=CELL_W; i<pParent->totalWidth; i+=CELL_W) /// TODO: i < pParent->coverX is faster
		fl_copy_offscreen(x()+i, y(), CELL_W, h(), surface2, 0, 0);
#endif

	baseDraw(false);
	draw_children();
}


/* -------------------------------------------------------------------------- */


int gePianoRoll::handle(int e)
{
	int ret = Fl_Group::handle(e);

	switch (e) {
		case FL_PUSH:	{

			/* avoid click on grey area */

			if (Fl::event_x() >= pParent->coverX) {
				ret = 1;
				break;
			}

			push_y = Fl::event_y() - y();

			if (Fl::event_button1()) {

				/* ax is driven by grid, ay by the height in px of each note */

				int ax = Fl::event_x();
				int ay = Fl::event_y();

				/* vertical snap */

				int edge = (ay-y()) % CELL_H;
				if (edge != 0) ay -= edge;

				/* If there are no pianoItems below the mouse, add a new one. Also check 
				that it doesn't overflow on the grey area, by shifting it to the left if
				necessary. */

				gePianoItem* pianoItem = dynamic_cast<gePianoItem*>(Fl::belowmouse());
				if (pianoItem == nullptr) {
					int greyover = ax+20 - pParent->coverX-x();
					if (greyover > 0)
						ax -= greyover;
					add(new gePianoItem(ax, ay, ax-x(), ay-y(), nullptr, nullptr, pParent));
					redraw();
				}
			}
			ret = 1;
			break;
		}
		case FL_DRAG:	{

			if (Fl::event_button3()) {

				geNoteEditor* prc = static_cast<geNoteEditor*>(parent());
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
		case FL_MOUSEWHEEL: {   // nothing to do, just avoid small internal scroll
			ret = 1;
			break;
		}
	}
	return ret;
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::updateActions()
{
	for (int k=0; k<children(); k++)
		static_cast<geBasePianoItem*>(child(k))->reposition(x());
}
