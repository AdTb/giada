/* ----------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiOutput
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_MIDI_OUTPUT_H
#define GD_MIDI_OUTPUT_H


#include <FL/Fl.H>
#include <stdint.h>
#include "ge_window.h"


/* There's no such thing as a gdMidiOutputMaster vs gdMidiOutputChannel. MIDI
output master is managed by the configuration window, hence gdMidiOutput deals
only with channels.

Both MidiOutputMidiCh and MidiOutputSampleCh have the MIDI lighting widget set.
In addition MidiOutputMidiCh has the MIDI message output box. */

class gdMidiOutput : public gWindow
{
protected:

	class gClick *ok;

	void stopMidiLearn(class gLearner *l);

	/* cb_learn
	 * callback attached to kernelMidi to learn various actions. */

	static void cb_learn  (uint32_t msg, void *data);
	inline void __cb_learn(uint32_t *param, uint32_t msg, class gLearner *l);

	static void cb_close  (Fl_Widget *w, void *p);
	inline void __cb_close();

public:

	gdMidiOutput();
};


/* -------------------------------------------------------------------------- */


class gdMidiOutputMidiCh : public gdMidiOutput
{
private:

	static void cb_save          (Fl_Widget *w, void *p);
	static void cb_cancel        (Fl_Widget *w, void *p);
	static void cb_enableChanList(Fl_Widget *w, void *p);
	inline void __cb_save          ();
	inline void __cb_cancel        ();
	inline void __cb_enableChanList();

	void fillChanMenu(class gChoice *m);

	class gCheck  *enableOut;
	class gChoice *chanListOut;
	class gButton *save;
	class gButton *cancel;

	class MidiChannel *ch;

public:

	gdMidiOutputMidiCh(class MidiChannel *ch);
};


/* -------------------------------------------------------------------------- */


class gdMidiOutputSampleCh : public gdMidiOutput
{
private:

	class gCheck *enable;

	class SampleChannel *ch;

	static void cb_enable  (Fl_Widget *w, void *p);
	inline void __cb_enable();

public:

	gdMidiOutputSampleCh(class SampleChannel *ch);
};

#endif
