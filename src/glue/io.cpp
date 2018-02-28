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


#include <FL/Fl.H>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../core/recorder.h"
#include "../core/kernelAudio.h"
#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/wave.h"
#include "../core/channel.h"
#include "../core/clock.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "main.h"
#include "channel.h"
#include "transport.h"
#include "io.h"


extern gdMainWindow* G_MainWin;


namespace giada {
namespace c     {
namespace io 
{
void keyPress(Channel* ch, bool ctrl, bool shift)
{
	if (ch->type == CHANNEL_SAMPLE)
		keyPress(static_cast<SampleChannel*>(ch), ctrl, shift);
	else
		keyPress(static_cast<MidiChannel*>(ch), ctrl, shift);
}


/* -------------------------------------------------------------------------- */


void keyRelease(Channel* ch, bool ctrl, bool shift)
{
	if (ch->type == CHANNEL_SAMPLE)
		keyRelease((SampleChannel*)ch, ctrl, shift);
}


/* -------------------------------------------------------------------------- */


void keyPress(MidiChannel* ch, bool ctrl, bool shift)
{
	if (ctrl)
		c::channel::toggleMute(ch);
	else
	if (shift)
		ch->kill(0);        // on frame 0: user-generated event
	else
		ch->start(0, true, m::clock::getQuantize(), m::clock::isRunning(), false, true); // on frame 0: user-generated event
}


/* -------------------------------------------------------------------------- */


void keyPress(SampleChannel* ch, bool ctrl, bool shift)
{
	/* case CTRL */

	if (ctrl)
		c::channel::toggleMute(ch);

	/* case SHIFT
	 *
	 * action recording on:
	 * 		if seq is playing, rec a killchan
	 * action recording off:
	 * 		if chan has recorded events:
	 * 		|	 if seq is playing OR channel 'c' is stopped, de/activate recs
	 * 		|	 else kill chan
	 *		else kill chan */

	else
	if (shift) {
		if (m::recorder::active) {
			if (m::clock::isRunning()) {
				ch->kill(0); // on frame 0: user-generated event
				if (m::recorder::canRec(ch, m::clock::isRunning(), m::mixer::recording) &&
            !(ch->mode & LOOP_ANY))
        {   // don't record killChan actions for LOOP channels
					m::recorder::rec(ch->index, G_ACTION_KILL, m::clock::getCurrentFrame());
          ch->hasActions = true;
        }
			}
		}
		else {
			if (ch->hasActions) {
				if (m::clock::isRunning() || ch->status == STATUS_OFF)
					ch->readActions ? c::channel::stopReadingRecs(ch) : c::channel::startReadingRecs(ch);
				else
					ch->kill(0);  // on frame 0: user-generated event
			}
			else
				ch->kill(0);    // on frame 0: user-generated event
		}
	}
	else {  /* case no modifier */

		/* record now if the quantizer is off, otherwise let mixer to handle it
		 * when a quantoWait has passed. Moreover, KEYPRESS and KEYREL are
		 * meaningless for loop modes */

		if (m::clock::getQuantize() == 0 &&
		    m::recorder::canRec(ch, m::clock::isRunning(), m::mixer::recording) &&
	      !(ch->mode & LOOP_ANY))
		{
			if (ch->mode == SINGLE_PRESS) {
				m::recorder::startOverdub(ch->index, G_ACTION_KEYS, m::clock::getCurrentFrame(),
          m::kernelAudio::getRealBufSize());
        ch->readActions = false;   // don't read actions while overdubbing
      }
			else {
				m::recorder::rec(ch->index, G_ACTION_KEYPRESS, m::clock::getCurrentFrame());
        ch->hasActions = true;

        /* Why return here? You record an action (as done on line 148) and then
				you call ch->start (line 165): Mixer, which is on another thread, reads
        your newly recorded action if you have readActions == true, and then
        ch->start kicks in right after it	(as done on	line 165).
				The result: Mixer plays the channel (due to the new action) but ch->start
				kills it right away (because the sample is playing). Fix: call ch->start
				only if you are not recording anything, i.e. let Mixer play it. */

        if (ch->readActions)
          return;
			}
		}

		/* This is a user-generated event, so it's on frame 0 */

		ch->start(0, true, m::clock::getQuantize(), m::clock::isRunning(), false, true);
	}

	/* the GUI update is done by gui_refresh() */
}


/* -------------------------------------------------------------------------- */


void keyRelease(SampleChannel* ch, bool ctrl, bool shift)
{
	using namespace giada::m;

	if (ctrl || shift)
		return;

	ch->stop();

	/* record a key release only if channel is single_press. For any
	 * other mode the KEY REL is meaningless. */

	if (ch->mode == SINGLE_PRESS && recorder::canRec(ch, clock::isRunning(), mixer::recording))
		recorder::stopOverdub(clock::getCurrentFrame(), clock::getTotalFrames(),
      &mixer::mutex_recs);

	/* the GUI update is done by gui_refresh() */

}


/* -------------------------------------------------------------------------- */


void startStopActionRec(bool gui)
{
	m::recorder::active ? stopActionRec(gui) : startActionRec(gui);
}


/* -------------------------------------------------------------------------- */


void startActionRec(bool gui)
{
	using namespace giada::m;

	if (kernelAudio::getStatus() == false)
		return;

	recorder::active = true;

	if (!clock::isRunning())
		glue_startSeq(false);  // update gui ayway

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(1);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void stopActionRec(bool gui)
{
	/* stop the recorder and sort new actions */

	m::recorder::active = false;
	m::recorder::sortActions();

	for (Channel* ch : m::mixer::channels)
	{
		if (ch->type == CHANNEL_MIDI)
			continue;
		SampleChannel* sch = static_cast<SampleChannel*>(ch);
		G_MainWin->keyboard->setChannelWithActions(static_cast<geSampleChannel*>(sch->guiChannel));
		if (!sch->readActions && sch->hasActions)
			c::channel::startReadingRecs(sch, false);
	}

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(0);
		Fl::unlock();
	}

	gu_refreshActionEditor();  // in case it's open
}


/* -------------------------------------------------------------------------- */


void startStopInputRec(bool gui)
{
	if (m::mixer::recording)
		stopInputRec(gui);
	else
	if (!startInputRec(gui))
		gdAlert("No channels armed/available for audio recording.");
}


/* -------------------------------------------------------------------------- */


int startInputRec(bool gui)
{
	using namespace giada::m;

	if (kernelAudio::getStatus() == false)
		return false;

	if (!mh::startInputRec()) {
	  Fl::lock();
	  G_MainWin->mainTransport->updateRecInput(0);  // set it off, anyway
		Fl::unlock();
		return false;
	}

	if (!clock::isRunning())
		glue_startSeq(false); // update gui anyway

  Fl::lock();
    if (!gui)
		  G_MainWin->mainTransport->updateRecInput(1);
    G_MainWin->mainTimer->setLock(true);
  Fl::unlock();

  /* Update sample name inside sample channels' main button. This is useless for
  midi channel, but let's do it anyway. */

  for (Channel* ch : m::mixer::channels)
    ch->guiChannel->update();

	return true;
}


/* -------------------------------------------------------------------------- */


int stopInputRec(bool gui)
{
	using namespace giada::m;
	
	mh::stopInputRec();

	/* Start all sample channels in loop mode that were armed, i.e. that were
	recording stuff and not yet in play. They are also started in force mode, i.e.
  they must start playing right away at the current frame, not at the next first
  beat. */

	for (Channel* ch : mixer::channels) {
		if (ch->type == CHANNEL_MIDI)
			continue;
		SampleChannel* sch = static_cast<SampleChannel*>(ch);
		if (sch->mode & (LOOP_ANY) && sch->status == STATUS_OFF && sch->isArmed())
			sch->start(clock::getCurrentFrame(), true, clock::getQuantize(),
        clock::isRunning(), true, true);
	}

  Fl::lock();
    if (!gui)
		  G_MainWin->mainTransport->updateRecInput(0);
    G_MainWin->mainTimer->setLock(false);
  Fl::unlock();

	return 1;
}

}}} // giada::c::io::