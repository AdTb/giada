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


#include "../../../utils/log.h"
#include "../../../utils/string.h"
#include "../../elems/midiLearner.h"
#include "midiOutputBase.h"


using std::string;
using namespace giada;


gdMidiOutputBase::gdMidiOutputBase(int w, int h)
	: gdWindow(w, h, "Midi Output Setup")
{
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::stopMidiLearn(geMidiLearner *learner)
{
	m::midiDispatcher::stopMidiLearn();
	learner->updateValue();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_learn(uint32_t* param, uint32_t msg, geMidiLearner* l)
{
	*param = msg;
	stopMidiLearn(l);
	gu_log("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_learn(uint32_t msg, void *d)
{
	geMidiLearner::cbData_t* data   = (geMidiLearner::cbData_t*) d;
	gdMidiOutputBase*        window = (gdMidiOutputBase*) data->window;
	geMidiLearner* learner = data->learner;
	uint32_t*      param   = learner->param;
	window->cb_learn(param, msg, learner);
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_close(Fl_Widget* w, void* p)  { ((gdMidiOutputBase*)p)->cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_close()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_enableLightning(Fl_Widget* w, void* p)
{
	((gdMidiOutputBase*)p)->cb_enableLightning();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_enableLightning() {}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::setTitle(int chanNum)
{
	string tmp = "MIDI Output Setup (channel " + u::string::iToString(chanNum) + ")"; 
	copy_label(tmp.c_str());
}
