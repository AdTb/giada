/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include "utils/string.h"
#include "core/midiDispatcher.h"
#include "core/midiEvent.h"
#include "glue/io.h"
#include "gui/dialogs/midiIO/midiInputBase.h"
#include "basics/boxtypes.h"
#include "basics/button.h"
#include "basics/box.h"
#include "midiLearner.h"


namespace giada {
namespace v 
{
geMidiLearner::geMidiLearner(int X, int Y, int W, const char* l, int param, uint32_t value)
: geMidiLearner(X, Y, W, l, param, value, 0)
{
}


geMidiLearner::geMidiLearner(int X, int Y, int W, const char* l, int param, uint32_t value, ID channelId)
: Fl_Group   (X, Y, W, 20),
  param      (param),
  m_channelId(channelId)
{
	begin();
	m_text     = new geBox(x(), y(), 156, 20, l);
	m_valueBtn = new geButton(m_text->x()+m_text->w()+4, y(), 80, 20);
	m_button   = new geButton(m_valueBtn->x()+m_valueBtn->w()+4, y(), 40, 20, "learn");
	end();

	m_text->box(G_CUSTOM_BORDER_BOX);
	m_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	m_valueBtn->box(G_CUSTOM_BORDER_BOX);
	m_valueBtn->callback(cb_value, (void*)this);
	m_valueBtn->when(FL_WHEN_RELEASE);

	m_button->type(FL_TOGGLE_BUTTON);
	m_button->callback(cb_button, (void*)this);
	
	refresh(value);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::refresh(uint32_t value)
{
	std::string tmp = "(not set)";
	
	if (value != 0x0) {
		tmp = "0x" + u::string::iToString(value, /*hex=*/true);
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
	}

	m_valueBtn->copy_label(tmp.c_str());
	m_button->value(0);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button(Fl_Widget* v, void* p) { ((geMidiLearner*)p)->cb_button(); }
void geMidiLearner::cb_value(Fl_Widget* v, void* p) { ((geMidiLearner*)p)->cb_value(); }


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_value()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE)
		c::io::clearMidiLearn(param, m_channelId);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button()
{
	if (m_button->value() == 1)
		c::io::startMidiLearn(param, m_channelId);
	else
		c::io::stopMidiLearn();
}
}} // giada::v::
