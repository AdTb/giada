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


#include "const.h"
#ifdef G_OS_MAC
	#include <RtMidi.h>
#else
	#include <rtmidi/RtMidi.h>
#endif
#include "../utils/log.h"
#include "../glue/channel.h"
#include "../glue/plugin.h"
#include "../glue/main.h"
#include "../glue/transport.h"
#include "../glue/io.h"
#include "mixer.h"
#include "channel.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "midiEvent.h"
#include "conf.h"
#include "midiMapConf.h"
#ifdef WITH_VST
	#include "pluginHost.h"
	#include "plugin.h"
#endif
#include "kernelMidi.h"


using std::string;
using std::vector;


namespace giada {
namespace m {
namespace kernelMidi
{
namespace
{
bool status = false;
int api = 0;
RtMidiOut* midiOut = nullptr;
RtMidiIn*  midiIn  = nullptr;
unsigned numOutPorts = 0;
unsigned numInPorts  = 0;

/* cb_learn
 * callback prepared by the gdMidiGrabber window and called by
 * kernelMidi. It contains things to do once the midi message has been
 * stored. */

cb_midiLearn* cb_learn = nullptr;
void* cb_data = nullptr;


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void processPlugins(Channel* ch, const MidiEvent& midiEvent)
{
	/* Pure value: if 'noNoteOff' in global config, get the raw value with the
	'velocy' byte. Otherwise strip it off. */

	uint32_t pure = midiEvent.getRaw(conf::noNoteOff);

	/* Plugins' parameters layout reflects the structure of the matrix
	Channel::midiInPlugins. It is safe to assume then that i (i.e. Plugin*) and k 
	indexes match both the structure of Channel::midiInPlugins and 
	vector<Plugin*>* plugins. */

	vector<Plugin*>* plugins = pluginHost::getStack(pluginHost::CHANNEL, ch);

	for (Plugin* plugin : *plugins) {
		for (unsigned k=0; k<plugin->midiInParams.size(); k++) {
			uint32_t midiInParam = plugin->midiInParams.at(k);
			if (pure != midiInParam)
				continue;
			float vf = midiEvent.getVelocity() / 127.0f;
			c::plugin::setParameter(plugin, k, vf, false); // false: not from GUI
			gu_log("  >>> [plugin %d parameter %d] ch=%d (pure=0x%X, value=%d, float=%f)\n",
				plugin->getId(), k, ch->index, pure, midiEvent.getVelocity(), vf);
		}
	}
}

#endif


/* -------------------------------------------------------------------------- */


void processChannels(const MidiEvent& midiEvent)
{
	/* Pure value: if 'noNoteOff' in global config, get the raw value with the
	'velocy' byte. Otherwise strip it off. */

	uint32_t pure = midiEvent.getRaw(conf::noNoteOff);

	for (Channel* ch : mixer::channels) {

		/* Do nothing on this channel if MIDI in is disabled or filtered out for
		the current MIDI channel. */

		if (!ch->midiIn || !ch->isMidiInAllowed(midiEvent.getChannel()))
			continue;

		if      (pure == ch->midiInKeyPress) {
			gu_log("  >>> keyPress, ch=%d (pure=0x%X)\n", ch->index, pure);
			glue_keyPress(ch, false, false);
		}
		else if (pure == ch->midiInKeyRel) {
			gu_log("  >>> keyRel ch=%d (pure=0x%X)\n", ch->index, pure);
			glue_keyRelease(ch, false, false);
		}
		else if (pure == ch->midiInMute) {
			gu_log("  >>> mute ch=%d (pure=0x%X)\n", ch->index, pure);
			glue_toggleMute(ch, false);
		}		
		else if (pure == ch->midiInKill) {
			gu_log("  >>> kill ch=%d (pure=0x%X)\n", ch->index, pure);
			glue_kill(ch);
		}		
		else if (pure == ch->midiInArm) {
			gu_log("  >>> arm ch=%d (pure=0x%X)\n", ch->index, pure);
			glue_toggleArm(ch, false);
		}
		else if (pure == ch->midiInSolo) {
			gu_log("  >>> solo ch=%d (pure=0x%X)\n", ch->index, pure);
			glue_toggleSolo(ch, false);
		}
		else if (pure == ch->midiInVolume) {
			float vf = midiEvent.getVelocity() / 127.0f;
			gu_log("  >>> volume ch=%d (pure=0x%X, value=%d, float=%f)\n",
				ch->index, pure, midiEvent.getVelocity(), vf);
			glue_setVolume(ch, vf, false);
		}
		else {
			SampleChannel* sch = static_cast<SampleChannel*>(ch);
			if (pure == sch->midiInPitch) {
				float vf = midiEvent.getVelocity() / (127/4.0f); // [0-127] ~> [0.0-4.0]
				gu_log("  >>> pitch ch=%d (pure=0x%X, value=%d, float=%f)\n",
					sch->index, pure, midiEvent.getVelocity(), vf);
				glue_setPitch(sch, vf);
			}
			else 
			if (pure == sch->midiInReadActions) {
				gu_log("  >>> toggle read actions ch=%d (pure=0x%X)\n", sch->index, pure);
				glue_toggleReadingRecs(sch, false);
			}
		}

#ifdef WITH_VST
		processPlugins(ch, midiEvent); // Process plugins' parameters
#endif

		/* Redirect full midi message (pure + velocity) to plugins. */

		ch->receiveMidi(midiEvent.getRaw());
	}
}


/* -------------------------------------------------------------------------- */


void processMaster(const MidiEvent& midiEvent)
{
	/* Pure value: if 'noNoteOff' in global config, get the raw value with the
	'velocy' byte. Otherwise strip it off. */

	uint32_t pure = midiEvent.getRaw(conf::noNoteOff);

	if      (pure == conf::midiInRewind) {
		gu_log("  >>> rewind (master) (pure=0x%X)\n", pure);
		glue_rewindSeq(false);
	}
	else if (pure == conf::midiInStartStop) {
		gu_log("  >>> startStop (master) (pure=0x%X)\n", pure);
		glue_startStopSeq(false);
	}
	else if (pure == conf::midiInActionRec) {
		gu_log("  >>> actionRec (master) (pure=0x%X)\n", pure);
		glue_startStopActionRec(false);
	}
	else if (pure == conf::midiInInputRec) {
		gu_log("  >>> inputRec (master) (pure=0x%X)\n", pure);
		glue_startStopInputRec(false);
	}
	else if (pure == conf::midiInMetronome) {
		gu_log("  >>> metronome (master) (pure=0x%X)\n", pure);
		glue_startStopMetronome(false);
	}
	else if (pure == conf::midiInVolumeIn) {
		float vf = midiEvent.getVelocity() / 127.0f;
		gu_log("  >>> input volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
		glue_setInVol(vf, false);
	}
	else if (pure == conf::midiInVolumeOut) {
		float vf = midiEvent.getVelocity() / 127.0f;
		gu_log("  >>> output volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
		glue_setOutVol(vf, false);
	}
	else if (pure == conf::midiInBeatDouble) {
		gu_log("  >>> sequencer x2 (master) (pure=0x%X)\n", pure);
		glue_beatsMultiply();
	}
	else if (pure == conf::midiInBeatHalf) {
		gu_log("  >>> sequencer /2 (master) (pure=0x%X)\n", pure);
		glue_beatsDivide();
	}
}


/* -------------------------------------------------------------------------- */


static void callback(double t, std::vector<unsigned char>* msg, void* data)
{
	/* For now we handle other MIDI signals (common and real-time messages) as 
	unknown, for debugging purposes. */

	if (msg->size() < 3) {
		//gu_log("[KM] MIDI received - unknown signal - size=%d, value=0x", (int) msg->size());
		//for (unsigned i=0; i<msg->size(); i++)
		//	gu_log("%X", (int) msg->at(i));
		//gu_log("\n");
		return;
	}

	/* Here we want to catch two things: a) note on/note off from a keyboard and 
	b) knob/wheel/slider movements from a controller. */

	MidiEvent midiEvent(msg->at(0), msg->at(1), msg->at(2));

	gu_log("[KM] MIDI received - 0x%X (chan %d)\n", midiEvent.getRaw(), 
		midiEvent.getChannel());

	/* Start dispatcher. If midi learn is on don't parse channels, just learn 
	incoming MIDI signal. Learn callback wants 'pure' MIDI event: if 'noNoteOff' 
	in global config, get the raw value with the 'velocy' byte. Otherwise strip it 
	off. If midi learn is off process master events first, then each channel 
	in the stack. This way incoming signals don't get processed by glue_* when 
	MIDI learning is on. */

	if (cb_learn)
		cb_learn(midiEvent.getRaw(conf::noNoteOff), cb_data);
	else {
		processMaster(midiEvent);
		processChannels(midiEvent);
	}
}


/* -------------------------------------------------------------------------- */


void sendMidiLightningInitMsgs()
{
	for(unsigned i=0; i<midimap::initCommands.size(); i++) {
		midimap::message_t msg = midimap::initCommands.at(i);
		if (msg.value != 0x0 && msg.channel != -1) {
			gu_log("[KM] MIDI send (init) - Channel %x - Event 0x%X\n", msg.channel, msg.value);
			send(msg.value | MIDI_CHANS[msg.channel]);
		}
	}
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void startMidiLearn(cb_midiLearn* cb, void* data)
{
	cb_learn = cb;
	cb_data  = data;
}


/* -------------------------------------------------------------------------- */


void stopMidiLearn()
{
	cb_learn = nullptr;
	cb_data  = nullptr;
}


/* -------------------------------------------------------------------------- */


void setApi(int _api)
{
	api = _api;
	gu_log("[KM] using system 0x%x\n", api);
}


/* -------------------------------------------------------------------------- */


int openOutDevice(int port)
{
	try {
		midiOut = new RtMidiOut((RtMidi::Api) api, "Giada MIDI Output");
		status = true;
	}
	catch (RtMidiError &error) {
		gu_log("[KM] MIDI out device error: %s\n", error.getMessage().c_str());
		status = false;
		return 0;
	}

	/* print output ports */

	numOutPorts = midiOut->getPortCount();
	gu_log("[KM] %d output MIDI ports found\n", numOutPorts);
	for (unsigned i=0; i<numOutPorts; i++)
		gu_log("  %d) %s\n", i, getOutPortName(i).c_str());

	/* try to open a port, if enabled */

	if (port != -1 && numOutPorts > 0) {
		try {
			midiOut->openPort(port, getOutPortName(port));
			gu_log("[KM] MIDI out port %d open\n", port);

			/* TODO - it shold send midiLightning message only if there is a map loaded
			and available in midimap:: */

			sendMidiLightningInitMsgs();
			return 1;
		}
		catch (RtMidiError &error) {
			gu_log("[KM] unable to open MIDI out port %d: %s\n", port, error.getMessage().c_str());
			status = false;
			return 0;
		}
	}
	else
		return 2;
}


/* -------------------------------------------------------------------------- */


int openInDevice(int port)
{
	try {
		midiIn = new RtMidiIn((RtMidi::Api) api, "Giada MIDI input");
		status = true;
	}
	catch (RtMidiError &error) {
		gu_log("[KM] MIDI in device error: %s\n", error.getMessage().c_str());
		status = false;
		return 0;
	}

	/* print input ports */

	numInPorts = midiIn->getPortCount();
	gu_log("[KM] %d input MIDI ports found\n", numInPorts);
	for (unsigned i=0; i<numInPorts; i++)
		gu_log("  %d) %s\n", i, getInPortName(i).c_str());

	/* try to open a port, if enabled */

	if (port != -1 && numInPorts > 0) {
		try {
			midiIn->openPort(port, getInPortName(port));
			midiIn->ignoreTypes(true, false, true); // ignore all system/time msgs, for now
			gu_log("[KM] MIDI in port %d open\n", port);
			midiIn->setCallback(&callback);
			return 1;
		}
		catch (RtMidiError &error) {
			gu_log("[KM] unable to open MIDI in port %d: %s\n", port, error.getMessage().c_str());
			status = false;
			return 0;
		}
	}
	else
		return 2;
}


/* -------------------------------------------------------------------------- */


bool hasAPI(int API)
{
	vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* -------------------------------------------------------------------------- */


string getOutPortName(unsigned p)
{
	try { return midiOut->getPortName(p); }
	catch (RtMidiError &error) { return ""; }
}

string getInPortName(unsigned p)
{
	try { return midiIn->getPortName(p); }
	catch (RtMidiError &error) { return ""; }
}


/* -------------------------------------------------------------------------- */


void send(uint32_t data)
{
	if (!status)
		return;

	vector<unsigned char> msg(1, getB1(data));
	msg.push_back(getB2(data));
	msg.push_back(getB3(data));

	midiOut->sendMessage(&msg);
	gu_log("[KM] send msg=0x%X (%X %X %X)\n", data, msg[0], msg[1], msg[2]);
}


/* -------------------------------------------------------------------------- */


void send(int b1, int b2, int b3)
{
	if (!status)
		return;

	vector<unsigned char> msg(1, b1);

	if (b2 != -1)
		msg.push_back(b2);
	if (b3 != -1)
		msg.push_back(b3);

	midiOut->sendMessage(&msg);
	//gu_log("[KM] send msg=(%X %X %X)\n", b1, b2, b3);
}


/* -------------------------------------------------------------------------- */


unsigned countInPorts()
{
	return numInPorts;
}


unsigned countOutPorts()
{
	return numOutPorts;
}


/* -------------------------------------------------------------------------- */


int getB1(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
int getB2(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
int getB3(uint32_t iValue) { return (iValue >> 8)  & 0xFF; }


uint32_t getIValue(int b1, int b2, int b3)
{
	return (b1 << 24) | (b2 << 16) | (b3 << 8) | (0x00);
}


/* -------------------------------------------------------------------------- */


uint32_t setChannel(uint32_t iValue, int channel)
{
	uint32_t chanMask = 0xF << 24;
	return (iValue & (~chanMask)) | (channel << 24);
}


/* -------------------------------------------------------------------------- */


bool getStatus()
{
	return status;
}

}}}; // giada::m::kernelMidi::
