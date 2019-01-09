/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include "../../utils/string.h"
#include "../dialogs/midiIO/midiInputBase.h"
#include "basics/boxtypes.h"
#include "basics/button.h"
#include "basics/box.h"
#include "midiLearner.h"


using std::string;
using namespace giada;


geMidiLearner::geMidiLearner(int X, int Y, int W, const char* l,
  m::midiDispatcher::cb_midiLearn* cb, uint32_t* param, m::Channel* ch)
: Fl_Group(X, Y, W, 20),
  callback(cb),
  ch      (ch),
  param   (param)
{
	begin();
	text   = new geBox(x(), y(), 156, 20, l);
	value  = new geButton(text->x()+text->w()+4, y(), 80, 20);
	button = new geButton(value->x()+value->w()+4, y(), 40, 20, "learn");
	end();

	text->box(G_CUSTOM_BORDER_BOX);
	text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	value->box(G_CUSTOM_BORDER_BOX);
	value->callback(cb_value, (void*)this);
	value->when(FL_WHEN_RELEASE);
	updateValue();

	button->type(FL_TOGGLE_BUTTON);
	button->callback(cb_button, (void*)this);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::updateValue()
{
	string tmp;
	if (*param != 0x0) {
		tmp = "0x" + u::string::iToString(*param, true); // true: hex mode
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
	}
	else
		tmp = "(not set)";
	value->copy_label(tmp.c_str());
	button->value(0);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button(Fl_Widget* v, void* p) { ((geMidiLearner*)p)->cb_button(); }
void geMidiLearner::cb_value(Fl_Widget* v, void* p) { ((geMidiLearner*)p)->cb_value(); }


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_value()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		*param = 0x0;
		updateValue();
	}
	/// TODO - elif (LEFT_MOUSE) : insert values by hand
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button()
{
	if (button->value() == 1) {
		cbData.window  = static_cast<gdMidiInputBase*>(parent()); // parent = gdMidiInput
		cbData.learner = this;
		cbData.channel = ch;
		m::midiDispatcher::startMidiLearn(callback, (void*)&cbData);
	}
	else
		m::midiDispatcher::stopMidiLearn();
}
