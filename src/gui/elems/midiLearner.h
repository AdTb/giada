/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_MIDI_LEARNER_H
#define GE_MIDI_LEARNER_H


#include <FL/Fl_Group.H>
#include "../../core/midiDispatcher.h"
#include "../../core/channel.h"


class gdMidiInputBase;
class geMidiLearner;
class geBox;
class geButton;


class geMidiLearner : public Fl_Group
{
private:

	/* callback
	Callback to pass to midiDispatcher. Requires two parameters:
	 * uint32_t msg - MIDI message
	 * void   *data - extra data */

	giada::m::midiDispatcher::cb_midiLearn* callback;

	/* Channel it belongs to. Might be nullptr if the learner comes from the MIDI
	input master window. */

	giada::m::Channel* ch;

	geBox* text;
	geButton* value;
	geButton* button;

	static void cb_button(Fl_Widget* v, void* p);
	static void cb_value (Fl_Widget* v, void* p);
	void cb_button();
	void cb_value();

public:

  /* cbData_t
  Struct we pass to midiDispatcher as extra parameter. */

  struct cbData_t
  {
		gdMidiInputBase*   window;
		geMidiLearner*     learner;
		giada::m::Channel* channel;
	} cbData;

	/* param
	 * pointer to ch->midiIn[value] */

	uint32_t* param;

	geMidiLearner(int x, int y, int w, const char* l, 
		giada::m::midiDispatcher::cb_midiLearn* cb, uint32_t* param, giada::m::Channel* ch);

	void updateValue();
};


#endif
