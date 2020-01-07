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


#ifndef G_CONF_H
#define G_CONF_H


#include <atomic>
#include <string>
#include "utils/gui.h"
#include "core/const.h"
#include "core/types.h"


namespace giada {
namespace m {
namespace conf
{
struct Conf
{
	int  logMode        = LOG_MODE_MUTE;
	int  soundSystem    = G_DEFAULT_SOUNDSYS;
	int  soundDeviceOut = G_DEFAULT_SOUNDDEV_OUT;
	int  soundDeviceIn  = G_DEFAULT_SOUNDDEV_IN;
	int  channelsOut    = 0;
	int  channelsIn     = 0;
	int  samplerate     = G_DEFAULT_SAMPLERATE;
	int  buffersize     = G_DEFAULT_BUFSIZE;
	bool limitOutput    = false;
	int  rsmpQuality    = 0;

	int         midiSystem  = 0;
	int         midiPortOut = G_DEFAULT_MIDI_PORT_OUT;
	int         midiPortIn  = G_DEFAULT_MIDI_PORT_IN;
	std::string midiMapPath = "";
	std::string lastFileMap = "";
	int         midiSync    = MIDI_SYNC_NONE;
	float       midiTCfps   = 25.0f;

	bool recsStopOnChanHalt    = false;
	bool chansStopOnSeqHalt    = false;
	bool treatRecsAsLoops      = false;
	bool inputMonitorDefaultOn = false;

	std::string pluginPath;
	std::string patchPath;
	std::string samplePath;

	int mainWindowX = u::gui::centerWindowX(G_MIN_GUI_WIDTH);
	int mainWindowY = u::gui::centerWindowY(G_MIN_GUI_HEIGHT);
	int mainWindowW = G_MIN_GUI_WIDTH;
	int mainWindowH = G_MIN_GUI_HEIGHT;

	int         browserX = u::gui::centerWindowX(G_DEFAULT_SUBWINDOW_W);
	int         browserY = u::gui::centerWindowY(G_DEFAULT_SUBWINDOW_H); 
	int         browserW = G_DEFAULT_SUBWINDOW_W; 
	int         browserH = G_DEFAULT_SUBWINDOW_H;
	int         browserPosition;
	int         browserLastValue;
	std::string browserLastPath;

	int actionEditorY       = u::gui::centerWindowY(G_DEFAULT_SUBWINDOW_H);
	int actionEditorX       = u::gui::centerWindowX(G_DEFAULT_SUBWINDOW_W);
	int actionEditorW       = G_DEFAULT_SUBWINDOW_W; 
	int actionEditorH       = G_DEFAULT_SUBWINDOW_H; 
	int actionEditorZoom    = 100;
	int actionEditorGridVal = 0;
	int actionEditorGridOn  = false;

	int sampleEditorX;
	int sampleEditorY;
	int sampleEditorW = G_DEFAULT_SUBWINDOW_W;
	int sampleEditorH = G_DEFAULT_SUBWINDOW_H;
	int sampleEditorGridVal = 0;
	int sampleEditorGridOn  = false;

	int midiInputX; 
	int midiInputY; 
	int midiInputW = G_DEFAULT_SUBWINDOW_W; 
	int midiInputH = G_DEFAULT_SUBWINDOW_H;

	int pianoRollY = -1;
	int pianoRollH = 422;

	int sampleActionEditorH = 40; 
	int velocityEditorH     = 40; 
	int envelopeEditorH     = 40; 

	int pluginListX;
	int pluginListY;

	int   recTriggerMode  = static_cast<int>(RecTriggerMode::NORMAL);
	float recTriggerLevel = G_DEFAULT_REC_TRIGGER_LEVEL;

#ifdef WITH_VST

	int pluginChooserX; 
	int pluginChooserY;
	int pluginChooserW   = G_DEFAULT_SUBWINDOW_W; 
	int pluginChooserH   = G_DEFAULT_SUBWINDOW_H;
	int pluginSortMethod = 0;

#endif
};


/* -------------------------------------------------------------------------- */


extern Conf conf;


/* -------------------------------------------------------------------------- */


void init();
bool read();
bool write();

/* isMidiAllowed
Given a MIDI channel 'c' tells whether this channel should be allowed to receive
and process MIDI events on MIDI channel 'c'. */

bool isMidiInAllowed(int c);

extern int  soundSystem;
extern int  soundDeviceOut;
extern int  soundDeviceIn;
extern int  channelsOut;
extern int  channelsIn;
extern int  samplerate;
extern int  buffersize;
extern bool limitOutput;
extern int  rsmpQuality;

extern int  midiSystem;
extern int  midiPortOut;
extern int  midiPortIn;
extern std::string midiMapPath;
extern std::string lastFileMap;
extern int   midiSync;  // see const.h
extern float midiTCfps;

extern std::atomic<bool>     midiIn;
extern std::atomic<int>      midiInFilter;
extern std::atomic<uint32_t> midiInRewind;
extern std::atomic<uint32_t> midiInStartStop;
extern std::atomic<uint32_t> midiInActionRec;
extern std::atomic<uint32_t> midiInInputRec;
extern std::atomic<uint32_t> midiInMetronome;
extern std::atomic<uint32_t> midiInVolumeIn;
extern std::atomic<uint32_t> midiInVolumeOut;
extern std::atomic<uint32_t> midiInBeatDouble;
extern std::atomic<uint32_t> midiInBeatHalf;

extern bool recsStopOnChanHalt;
extern bool chansStopOnSeqHalt;
extern bool treatRecsAsLoops;
extern bool inputMonitorDefaultOn;

extern std::string pluginPath;
extern std::string patchPath;
extern std::string samplePath;

extern int mainWindowX, mainWindowY, mainWindowW, mainWindowH;

extern int browserX, browserY, browserW, browserH, browserPosition, browserLastValue;
extern std::string browserLastPath;

extern int actionEditorX, actionEditorY, actionEditorW, actionEditorH, actionEditorZoom;
extern int actionEditorGridVal;
extern int actionEditorGridOn;

extern int sampleEditorX, sampleEditorY, sampleEditorW, sampleEditorH;
extern int sampleEditorGridVal;
extern int sampleEditorGridOn;

extern int midiInputX, midiInputY, midiInputW, midiInputH;

extern int pianoRollY, pianoRollH;
extern int sampleActionEditorH; 
extern int velocityEditorH; 
extern int envelopeEditorH; 
extern int pluginListX, pluginListY;

extern int   recTriggerMode;
extern float recTriggerLevel;

#ifdef WITH_VST

extern int pluginChooserX, pluginChooserY, pluginChooserW, pluginChooserH;
extern int pluginSortMethod;

#endif
}}}; // giada::m::conf::

#endif
