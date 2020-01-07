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


#include <fstream>
#include <cassert>
#include <string>
#include <FL/Fl.H>
#include "deps/json/single_include/nlohmann/json.hpp"
#include "utils/fs.h"
#include "utils/log.h"
#include "core/const.h"
#include "core/types.h"
#include "conf.h"


namespace nl = nlohmann;


namespace giada {
namespace m {
namespace conf
{
namespace
{
std::string confFilePath = "";
std::string confDirPath  = "";


/* -------------------------------------------------------------------------- */

/* sanitize
Avoids funky values from config file. */

void sanitize_()
{
	if (!(conf.soundSystem & G_SYS_API_ANY)) conf.soundSystem = G_DEFAULT_SOUNDSYS;
	if (soundDeviceOut < 0) soundDeviceOut = G_DEFAULT_SOUNDDEV_OUT;
	if (soundDeviceIn < -1) soundDeviceIn = G_DEFAULT_SOUNDDEV_IN;
	if (channelsOut < 0) channelsOut = 0;
	if (channelsIn < 0)  channelsIn  = 0;
	if (buffersize < G_MIN_BUF_SIZE || buffersize > G_MAX_BUF_SIZE) buffersize = G_DEFAULT_BUFSIZE;
	if (midiPortOut < -1) midiPortOut = G_DEFAULT_MIDI_SYSTEM;
	if (midiPortOut < -1) midiPortOut = G_DEFAULT_MIDI_PORT_OUT;
	if (midiPortIn < -1) midiPortIn = G_DEFAULT_MIDI_PORT_IN;
	if (browserX < 0) browserX = 0;
	if (browserY < 0) browserY = 0;
	if (browserW < 396) browserW = 396;
	if (browserH < 302) browserH = 302;
	if (actionEditorX < 0) actionEditorX = 0;
	if (actionEditorY < 0) actionEditorY = 0;
	if (actionEditorW < 640) actionEditorW = 640;
	if (actionEditorH < 176) actionEditorH = 176;
	if (actionEditorZoom < 100) actionEditorZoom = 100;
	if (actionEditorGridVal < 0 || actionEditorGridVal > G_MAX_GRID_VAL) actionEditorGridVal = 0;
	if (actionEditorGridOn < 0) actionEditorGridOn = 0;
	if (pianoRollH <= 0) pianoRollH = 422;
	if (sampleActionEditorH <= 0) sampleActionEditorH = 40;
	if (velocityEditorH <= 0) velocityEditorH = 40;
	if (envelopeEditorH <= 0) envelopeEditorH = 40;
	if (sampleEditorX < 0) sampleEditorX = 0;
	if (sampleEditorY < 0) sampleEditorY = 0;
	if (sampleEditorW < 500) sampleEditorW = 500;
	if (sampleEditorH < 292) sampleEditorH = 292;
	if (sampleEditorGridVal < 0 || sampleEditorGridVal > G_MAX_GRID_VAL) sampleEditorGridVal = 0;
	if (sampleEditorGridOn < 0) sampleEditorGridOn = 0;
	if (midiInputX < 0) midiInputX = 0;
	if (midiInputY < 0) midiInputY = 0;
	if (midiInputW < 640) midiInputW = 640;
	if (midiInputH < 480) midiInputH = 480;
	if (pluginListX < 0) pluginListX = 0;
	if (pluginListY < 0) pluginListY = 0;
#ifdef WITH_VST
	if (pluginChooserW < 640) pluginChooserW = 640;
	if (pluginChooserH < 480) pluginChooserH = 480;
#endif
	if (samplerate < 8000) samplerate = G_DEFAULT_SAMPLERATE;
	if (rsmpQuality < 0 || rsmpQuality > 4) rsmpQuality = 0;
}


/* -------------------------------------------------------------------------- */


/* createConfigFolder
Creates local folder where to put the configuration file. Path differs from OS
to OS. */

int createConfigFolder_()
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	if (u::fs::dirExists(confDirPath))
		return 1;

	u::log::print("[conf::createConfigFolder] .giada folder not present. Updating...\n");

	if (u::fs::mkdir(confDirPath)) {
		u::log::print("[conf::createConfigFolder] status: ok\n");
		return 1;
	}
	else {
		u::log::print("[conf::createConfigFolder] status: error!\n");
		return 0;
	}

#else // Windows: nothing to do

	return 1;

#endif
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Conf conf;

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

/* TODO - move these into a RCUList (Engine) */

std::atomic<bool>     midiIn          (false);
std::atomic<int>      midiInFilter    (-1);
std::atomic<uint32_t> midiInRewind    (0x0);
std::atomic<uint32_t> midiInStartStop (0x0);
std::atomic<uint32_t> midiInActionRec (0x0);
std::atomic<uint32_t> midiInInputRec  (0x0);
std::atomic<uint32_t> midiInVolumeIn  (0x0);
std::atomic<uint32_t> midiInVolumeOut (0x0);
std::atomic<uint32_t> midiInBeatDouble(0x0);
std::atomic<uint32_t> midiInBeatHalf  (0x0);
std::atomic<uint32_t> midiInMetronome (0x0);

bool recsStopOnChanHalt    = false;
bool chansStopOnSeqHalt    = false;
bool treatRecsAsLoops      = false;
bool inputMonitorDefaultOn = false;

std::string pluginPath = "";
std::string patchPath  = "";
std::string samplePath = "";

int mainWindowX = (Fl::w() / 2) - (G_MIN_GUI_WIDTH / 2);
int mainWindowY = (Fl::h() / 2) - (G_MIN_GUI_HEIGHT / 2);
int mainWindowW = G_MIN_GUI_WIDTH;
int mainWindowH = G_MIN_GUI_HEIGHT;

int         browserX         = 0;
int         browserY         = 0;
int         browserW         = 640;
int         browserH         = 480;
int         browserPosition  = 0;
int         browserLastValue = 0;
std::string browserLastPath = "";

int actionEditorX       = 0;
int actionEditorY       = 0;
int actionEditorW       = 640;
int actionEditorH       = 480;
int actionEditorZoom    = 100;
int actionEditorGridVal = 1;
int actionEditorGridOn  = false;

int sampleEditorX = 0;
int sampleEditorY = 0;
int sampleEditorW = 640;
int sampleEditorH = 480;
int sampleEditorGridVal = 0;
int sampleEditorGridOn  = false;

int midiInputX = 0;
int midiInputY = 0;
int midiInputW = 640;
int midiInputH = 480;

int pianoRollY = -1;
int pianoRollH = 422;

int sampleActionEditorH = 40; 
int velocityEditorH     = 40; 
int envelopeEditorH     = 40; 

int pluginListX = 0;
int pluginListY = 0;

int   recTriggerMode  = static_cast<int>(RecTriggerMode::NORMAL);
float recTriggerLevel = G_DEFAULT_REC_TRIGGER_LEVEL;

#ifdef WITH_VST

int pluginChooserX   = 0;
int pluginChooserY   = 0;
int pluginChooserW   = 640;
int pluginChooserH   = 480;
int pluginSortMethod = 0;

#endif


/* -------------------------------------------------------------------------- */


void init()
{
	conf = Conf();

	/* Initialize confFilePath, i.e. the configuration file. In windows it is in
	 * the same dir of the .exe, while in Linux and OS X in ~/.giada */

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	confFilePath = u::fs::getHomePath() + G_SLASH + CONF_FILENAME;
	confDirPath  = u::fs::getHomePath() + G_SLASH;

#elif defined(_WIN32)

	confFilePath = CONF_FILENAME;
	confDirPath  = "";

#endif
}


/* -------------------------------------------------------------------------- */


bool isMidiInAllowed(int c)
{
	return midiInFilter == -1 || midiInFilter == c;
}


/* -------------------------------------------------------------------------- */


bool read()
{
	init();

	std::ifstream ifs(confFilePath);
	if (!ifs.good())
		return false;

	nl::json j = nl::json::parse(ifs);

	conf.logMode               = j[CONF_KEY_LOG_MODE];
	conf.soundSystem           = j[CONF_KEY_SOUND_SYSTEM];
	conf.soundDeviceOut        = j[CONF_KEY_SOUND_DEVICE_OUT];
	conf.soundDeviceIn         = j[CONF_KEY_SOUND_DEVICE_IN];
	conf.channelsOut           = j[CONF_KEY_CHANNELS_OUT];
	conf.channelsIn            = j[CONF_KEY_CHANNELS_IN];
	conf.samplerate            = j[CONF_KEY_SAMPLERATE];
	conf.buffersize            = j[CONF_KEY_BUFFER_SIZE];
	conf.limitOutput           = j[CONF_KEY_LIMIT_OUTPUT];
	conf.rsmpQuality           = j[CONF_KEY_RESAMPLE_QUALITY];
	conf.midiSystem            = j[CONF_KEY_MIDI_SYSTEM];
	conf.midiPortOut           = j[CONF_KEY_MIDI_PORT_OUT];
	conf.midiPortIn            = j[CONF_KEY_MIDI_PORT_IN];
	conf.midiMapPath           = j[CONF_KEY_MIDIMAP_PATH];
	conf.lastFileMap           = j[CONF_KEY_LAST_MIDIMAP];
	conf.midiSync              = j[CONF_KEY_MIDI_SYNC];
	conf.midiTCfps             = j[CONF_KEY_MIDI_TC_FPS];
	/*
	conf.midiIn                = j[CONF_KEY_MIDI_IN];
	conf.midiInFilter          = j[CONF_KEY_MIDI_IN_FILTER]; 
	conf.midiInRewind          = j[CONF_KEY_MIDI_IN_REWIND];
	conf.midiInStartStop       = j[CONF_KEY_MIDI_IN_START_STOP];
	conf.midiInActionRec       = j[CONF_KEY_MIDI_IN_ACTION_REC];
	conf.midiInInputRec        = j[CONF_KEY_MIDI_IN_INPUT_REC];
	conf.midiInMetronome       = j[CONF_KEY_MIDI_IN_METRONOME];
	conf.midiInVolumeIn        = j[CONF_KEY_MIDI_IN_VOLUME_IN];
	conf.midiInVolumeOut       = j[CONF_KEY_MIDI_IN_VOLUME_OUT];
	conf.midiInBeatDouble      = j[CONF_KEY_MIDI_IN_BEAT_DOUBLE];
	conf.midiInBeatHalf        = j[CONF_KEY_MIDI_IN_BEAT_HALF];
	*/
	conf.recsStopOnChanHalt    = j[CONF_KEY_RECS_STOP_ON_CHAN_HALT];
	conf.chansStopOnSeqHalt    = j[CONF_KEY_CHANS_STOP_ON_SEQ_HALT];
	conf.treatRecsAsLoops      = j[CONF_KEY_TREAT_RECS_AS_LOOPS];
	conf.inputMonitorDefaultOn = j[CONF_KEY_INPUT_MONITOR_DEFAULT_ON];
	conf.pluginPath            = j[CONF_KEY_PLUGINS_PATH];
	conf.patchPath             = j[CONF_KEY_PATCHES_PATH];
	conf.samplePath            = j[CONF_KEY_SAMPLES_PATH];
	conf.mainWindowX           = j[CONF_KEY_MAIN_WINDOW_X];
	conf.mainWindowY           = j[CONF_KEY_MAIN_WINDOW_Y];
	conf.mainWindowW           = j[CONF_KEY_MAIN_WINDOW_W];
	conf.mainWindowH           = j[CONF_KEY_MAIN_WINDOW_H];
	conf.browserX              = j[CONF_KEY_BROWSER_X];
	conf.browserY              = j[CONF_KEY_BROWSER_Y];
	conf.browserW              = j[CONF_KEY_BROWSER_W];
	conf.browserH              = j[CONF_KEY_BROWSER_H];
	conf.browserPosition       = j[CONF_KEY_BROWSER_POSITION];
	conf.browserLastPath       = j[CONF_KEY_BROWSER_LAST_PATH];
	conf.browserLastValue      = j[CONF_KEY_BROWSER_LAST_VALUE];
	conf.actionEditorX         = j[CONF_KEY_ACTION_EDITOR_X];
	conf.actionEditorY         = j[CONF_KEY_ACTION_EDITOR_Y];
	conf.actionEditorW         = j[CONF_KEY_ACTION_EDITOR_W];
	conf.actionEditorH         = j[CONF_KEY_ACTION_EDITOR_H];
	conf.actionEditorZoom      = j[CONF_KEY_ACTION_EDITOR_ZOOM];
	conf.actionEditorGridVal   = j[CONF_KEY_ACTION_EDITOR_GRID_VAL];
	conf.actionEditorGridOn    = j[CONF_KEY_ACTION_EDITOR_GRID_ON];
	conf.sampleEditorX         = j[CONF_KEY_SAMPLE_EDITOR_X];
	conf.sampleEditorY         = j[CONF_KEY_SAMPLE_EDITOR_Y];
	conf.sampleEditorW         = j[CONF_KEY_SAMPLE_EDITOR_W];
	conf.sampleEditorH         = j[CONF_KEY_SAMPLE_EDITOR_H];
	conf.sampleEditorGridVal   = j[CONF_KEY_SAMPLE_EDITOR_GRID_VAL];
	conf.sampleEditorGridOn    = j[CONF_KEY_SAMPLE_EDITOR_GRID_ON];
	conf.pianoRollY            = j[CONF_KEY_PIANO_ROLL_Y];
	conf.pianoRollH            = j[CONF_KEY_PIANO_ROLL_H];
	conf.sampleActionEditorH  =  j[CONF_KEY_SAMPLE_ACTION_EDITOR_H];
	conf.velocityEditorH       = j[CONF_KEY_VELOCITY_EDITOR_H];
	conf.envelopeEditorH       = j[CONF_KEY_ENVELOPE_EDITOR_H];
	conf.pluginListX           = j[CONF_KEY_PLUGIN_LIST_X];
	conf.pluginListY           = j[CONF_KEY_PLUGIN_LIST_Y];
	conf.midiInputX            = j[CONF_KEY_MIDI_INPUT_X];
	conf.midiInputY            = j[CONF_KEY_MIDI_INPUT_Y];
	conf.midiInputW            = j[CONF_KEY_MIDI_INPUT_W];
	conf.midiInputH            = j[CONF_KEY_MIDI_INPUT_H];
	conf.recTriggerMode        = j[CONF_KEY_REC_TRIGGER_MODE];
	conf.recTriggerLevel       = j[CONF_KEY_REC_TRIGGER_LEVEL];
#ifdef WITH_VST
	conf.pluginChooserX        = j[CONF_KEY_PLUGIN_CHOOSER_X];
	conf.pluginChooserY        = j[CONF_KEY_PLUGIN_CHOOSER_Y];
	conf.pluginChooserW        = j[CONF_KEY_PLUGIN_CHOOSER_W];
	conf.pluginChooserH        = j[CONF_KEY_PLUGIN_CHOOSER_H];
	conf.pluginSortMethod      = j[CONF_KEY_PLUGIN_SORT_METHOD];
#endif

	sanitize_();

	return true;
}


/* -------------------------------------------------------------------------- */


bool write()
{
	if (!createConfigFolder_())
		return false;

	nl::json j;

	j[CONF_KEY_HEADER] =                    "GIADACFG";
	j[CONF_KEY_LOG_MODE] =                  conf.logMode;
	j[CONF_KEY_SOUND_SYSTEM] =              conf.soundSystem;
	j[CONF_KEY_SOUND_DEVICE_OUT] =          conf.soundDeviceOut;
	j[CONF_KEY_SOUND_DEVICE_IN] =           conf.soundDeviceIn;
	j[CONF_KEY_CHANNELS_OUT] =              conf.channelsOut;
	j[CONF_KEY_CHANNELS_IN] =               conf.channelsIn;
	j[CONF_KEY_SAMPLERATE] =                conf.samplerate;
	j[CONF_KEY_BUFFER_SIZE] =               conf.buffersize;
	j[CONF_KEY_LIMIT_OUTPUT] =              conf.limitOutput;
	j[CONF_KEY_RESAMPLE_QUALITY] =          conf.rsmpQuality;
	j[CONF_KEY_MIDI_SYSTEM] =               conf.midiSystem;
	j[CONF_KEY_MIDI_PORT_OUT] =             conf.midiPortOut;
	j[CONF_KEY_MIDI_PORT_IN] =              conf.midiPortIn;
	j[CONF_KEY_MIDIMAP_PATH] =              conf.midiMapPath;
	j[CONF_KEY_LAST_MIDIMAP] =              conf.lastFileMap;
	j[CONF_KEY_MIDI_SYNC] =                 conf.midiSync;
	j[CONF_KEY_MIDI_TC_FPS] =               conf.midiTCfps;
	/*
	j[CONF_KEY_MIDI_IN] =                   midiIn.load();
	j[CONF_KEY_MIDI_IN_FILTER] =            midiInFilter.load();
	j[CONF_KEY_MIDI_IN_REWIND] =            midiInRewind.load();
	j[CONF_KEY_MIDI_IN_START_STOP] =        midiInStartStop.load();
	j[CONF_KEY_MIDI_IN_ACTION_REC] =        midiInActionRec.load();
	j[CONF_KEY_MIDI_IN_INPUT_REC] =         midiInInputRec.load();
	j[CONF_KEY_MIDI_IN_METRONOME] =         midiInMetronome.load();
	j[CONF_KEY_MIDI_IN_VOLUME_IN] =         midiInVolumeIn.load();
	j[CONF_KEY_MIDI_IN_VOLUME_OUT] =        midiInVolumeOut.load();
	j[CONF_KEY_MIDI_IN_BEAT_DOUBLE] =       midiInBeatDouble.load();
	j[CONF_KEY_MIDI_IN_BEAT_HALF] =         midiInBeatHalf.load();
	*/
	j[CONF_KEY_RECS_STOP_ON_CHAN_HALT]    = conf.recsStopOnChanHalt;
	j[CONF_KEY_CHANS_STOP_ON_SEQ_HALT]    = conf.chansStopOnSeqHalt;
	j[CONF_KEY_TREAT_RECS_AS_LOOPS]       = conf.treatRecsAsLoops;
	j[CONF_KEY_INPUT_MONITOR_DEFAULT_ON]  = conf.inputMonitorDefaultOn;
	j[CONF_KEY_PLUGINS_PATH]              = conf.pluginPath;
	j[CONF_KEY_PATCHES_PATH]              = conf.patchPath;
	j[CONF_KEY_SAMPLES_PATH]              = conf.samplePath;
	j[CONF_KEY_MAIN_WINDOW_X]             = conf.mainWindowX;
	j[CONF_KEY_MAIN_WINDOW_Y]             = conf.mainWindowY;
	j[CONF_KEY_MAIN_WINDOW_W]             = conf.mainWindowW;
	j[CONF_KEY_MAIN_WINDOW_H]             = conf.mainWindowH;
	j[CONF_KEY_BROWSER_X]                 = conf.browserX;
	j[CONF_KEY_BROWSER_Y]                 = conf.browserY;
	j[CONF_KEY_BROWSER_W]                 = conf.browserW;
	j[CONF_KEY_BROWSER_H]                 = conf.browserH;
	j[CONF_KEY_BROWSER_POSITION]          = conf.browserPosition;
	j[CONF_KEY_BROWSER_LAST_PATH]         = conf.browserLastPath;
	j[CONF_KEY_BROWSER_LAST_VALUE]        = conf.browserLastValue;
	j[CONF_KEY_ACTION_EDITOR_X]           = conf.actionEditorX;
	j[CONF_KEY_ACTION_EDITOR_Y]           = conf.actionEditorY;
	j[CONF_KEY_ACTION_EDITOR_W]           = conf.actionEditorW;
	j[CONF_KEY_ACTION_EDITOR_H]           = conf.actionEditorH;
	j[CONF_KEY_ACTION_EDITOR_ZOOM]        = conf.actionEditorZoom;
	j[CONF_KEY_ACTION_EDITOR_GRID_VAL]    = conf.actionEditorGridVal;
	j[CONF_KEY_ACTION_EDITOR_GRID_ON]     = conf.actionEditorGridOn;
	j[CONF_KEY_SAMPLE_EDITOR_X]           = conf.sampleEditorX;
	j[CONF_KEY_SAMPLE_EDITOR_Y]           = conf.sampleEditorY;
	j[CONF_KEY_SAMPLE_EDITOR_W]           = conf.sampleEditorW;
	j[CONF_KEY_SAMPLE_EDITOR_H]           = conf.sampleEditorH;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_VAL]    = conf.sampleEditorGridVal;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_ON]     = conf.sampleEditorGridOn;
	j[CONF_KEY_PIANO_ROLL_Y]              = conf.pianoRollY;
	j[CONF_KEY_PIANO_ROLL_H]              = conf.pianoRollH;
	j[CONF_KEY_SAMPLE_ACTION_EDITOR_H]    = conf.sampleActionEditorH;
	j[CONF_KEY_VELOCITY_EDITOR_H]         = conf.velocityEditorH;
	j[CONF_KEY_ENVELOPE_EDITOR_H]         = conf.envelopeEditorH;
	j[CONF_KEY_PLUGIN_LIST_X]             = conf.pluginListX;
	j[CONF_KEY_PLUGIN_LIST_Y]             = conf.pluginListY;
	j[CONF_KEY_MIDI_INPUT_X]              = conf.midiInputX;
	j[CONF_KEY_MIDI_INPUT_Y]              = conf.midiInputY;
	j[CONF_KEY_MIDI_INPUT_W]              = conf.midiInputW;
	j[CONF_KEY_MIDI_INPUT_H]              = conf.midiInputH;
	j[CONF_KEY_REC_TRIGGER_MODE]          = conf.recTriggerMode;
	j[CONF_KEY_REC_TRIGGER_LEVEL]         = conf.recTriggerLevel;
#ifdef WITH_VST
	j[CONF_KEY_PLUGIN_CHOOSER_X]          = conf.pluginChooserX;
	j[CONF_KEY_PLUGIN_CHOOSER_Y]          = conf.pluginChooserY;
	j[CONF_KEY_PLUGIN_CHOOSER_W]          = conf.pluginChooserW;
	j[CONF_KEY_PLUGIN_CHOOSER_H]          = conf.pluginChooserH;
	j[CONF_KEY_PLUGIN_SORT_METHOD]        = conf.pluginSortMethod;
#endif

    std::ofstream ofs(confFilePath);
	if (!ofs.good()) {
		u::log::print("[conf::write] unable to write configuration file!\n");
		return false;
	}

    ofs << j;
	return true;
}
}}}; // giada::m::conf::