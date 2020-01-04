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

void sanitize()
{
	if (!(soundSystem & G_SYS_API_ANY)) soundSystem = G_DEFAULT_SOUNDSYS;
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
	if (midiInputW < G_DEFAULT_MIDI_INPUT_UI_W) midiInputW = G_DEFAULT_MIDI_INPUT_UI_W;
	if (midiInputH < G_DEFAULT_MIDI_INPUT_UI_H) midiInputH = G_DEFAULT_MIDI_INPUT_UI_H;
	if (configX < 0) configX = 0;
	if (configY < 0) configY = 0;
	if (pluginListX < 0) pluginListX = 0;
	if (pluginListY < 0) pluginListY = 0;
#ifdef WITH_VST
	if (pluginChooserW < 640) pluginChooserW = 640;
	if (pluginChooserH < 480) pluginChooserW = 480;
#endif
	if (bpmX < 0) bpmX = 0;
	if (bpmY < 0) bpmY = 0;
	if (beatsX < 0) beatsX = 0;
	if (beatsY < 0) beatsY = 0;
	if (aboutX < 0) aboutX = 0;
	if (aboutY < 0) aboutY = 0;
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


std::string header = "GIADACFG";

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
int midiInputW = G_DEFAULT_MIDI_INPUT_UI_W;
int midiInputH = G_DEFAULT_MIDI_INPUT_UI_H;

int pianoRollY = -1;
int pianoRollH = 422;

int sampleActionEditorH = 40; 
int velocityEditorH     = 40; 
int envelopeEditorH     = 40; 

int pluginListX = 0;
int pluginListY = 0;

int configX = 0;
int configY = 0;

int bpmX = 0;
int bpmY = 0;

int beatsX = 0;
int beatsY = 0;

int aboutX = 0;
int aboutY = 0;

int nameX = 0;
int nameY = 0;

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

	header           = j[CONF_KEY_HEADER];
	logMode          = j[CONF_KEY_LOG_MODE];
	soundSystem      = j[CONF_KEY_SOUND_SYSTEM];
	soundDeviceOut   = j[CONF_KEY_SOUND_DEVICE_OUT];
	soundDeviceIn    = j[CONF_KEY_SOUND_DEVICE_IN];
	channelsOut      = j[CONF_KEY_CHANNELS_OUT];
	channelsIn       = j[CONF_KEY_CHANNELS_IN];
	samplerate       = j[CONF_KEY_SAMPLERATE];
	buffersize       = j[CONF_KEY_BUFFER_SIZE];
	limitOutput      = j[CONF_KEY_LIMIT_OUTPUT];
	rsmpQuality      = j[CONF_KEY_RESAMPLE_QUALITY];
	midiSystem       = j[CONF_KEY_MIDI_SYSTEM];
	midiPortOut      = j[CONF_KEY_MIDI_PORT_OUT];
	midiPortIn       = j[CONF_KEY_MIDI_PORT_IN];
	midiMapPath      = j[CONF_KEY_MIDIMAP_PATH];
	lastFileMap      = j[CONF_KEY_LAST_MIDIMAP];
	midiSync         = j[CONF_KEY_MIDI_SYNC];
	midiTCfps        = j[CONF_KEY_MIDI_TC_FPS];
	midiIn           = j[CONF_KEY_MIDI_IN];
	midiInFilter     = j[CONF_KEY_MIDI_IN_FILTER]; 
	midiInRewind     = j[CONF_KEY_MIDI_IN_REWIND];
	midiInStartStop  = j[CONF_KEY_MIDI_IN_START_STOP];
	midiInActionRec  = j[CONF_KEY_MIDI_IN_ACTION_REC];
	midiInInputRec   = j[CONF_KEY_MIDI_IN_INPUT_REC];
	midiInMetronome  = j[CONF_KEY_MIDI_IN_METRONOME];
	midiInVolumeIn   = j[CONF_KEY_MIDI_IN_VOLUME_IN];
	midiInVolumeOut  = j[CONF_KEY_MIDI_IN_VOLUME_OUT];
	midiInBeatDouble = j[CONF_KEY_MIDI_IN_BEAT_DOUBLE];
	midiInBeatHalf   = j[CONF_KEY_MIDI_IN_BEAT_HALF];
	recsStopOnChanHalt    = j[CONF_KEY_RECS_STOP_ON_CHAN_HALT];
	chansStopOnSeqHalt    = j[CONF_KEY_CHANS_STOP_ON_SEQ_HALT];
	treatRecsAsLoops      = j[CONF_KEY_TREAT_RECS_AS_LOOPS];
	inputMonitorDefaultOn = j[CONF_KEY_INPUT_MONITOR_DEFAULT_ON];
	pluginPath  = j[CONF_KEY_PLUGINS_PATH];
	patchPath   = j[CONF_KEY_PATCHES_PATH];
	samplePath  = j[CONF_KEY_SAMPLES_PATH];
	mainWindowX = j[CONF_KEY_MAIN_WINDOW_X];
	mainWindowY = j[CONF_KEY_MAIN_WINDOW_Y];
	mainWindowW = j[CONF_KEY_MAIN_WINDOW_W];
	mainWindowH = j[CONF_KEY_MAIN_WINDOW_H];
	browserX = j[CONF_KEY_BROWSER_X];
	browserY = j[CONF_KEY_BROWSER_Y];
	browserW = j[CONF_KEY_BROWSER_W];
	browserH = j[CONF_KEY_BROWSER_H];
	browserPosition  = j[CONF_KEY_BROWSER_POSITION];
	browserLastPath  = j[CONF_KEY_BROWSER_LAST_PATH];
	browserLastValue = j[CONF_KEY_BROWSER_LAST_VALUE];
	actionEditorX = j[CONF_KEY_ACTION_EDITOR_X];
	actionEditorY = j[CONF_KEY_ACTION_EDITOR_Y];
	actionEditorW = j[CONF_KEY_ACTION_EDITOR_W];
	actionEditorH = j[CONF_KEY_ACTION_EDITOR_H];
	actionEditorZoom    = j[CONF_KEY_ACTION_EDITOR_ZOOM];
	actionEditorGridVal = j[CONF_KEY_ACTION_EDITOR_GRID_VAL];
	actionEditorGridOn  = j[CONF_KEY_ACTION_EDITOR_GRID_ON];
	sampleEditorX = j[CONF_KEY_SAMPLE_EDITOR_X];
	sampleEditorY = j[CONF_KEY_SAMPLE_EDITOR_Y];
	sampleEditorW = j[CONF_KEY_SAMPLE_EDITOR_W];
	sampleEditorH = j[CONF_KEY_SAMPLE_EDITOR_H];
	sampleEditorGridVal = j[CONF_KEY_SAMPLE_EDITOR_GRID_VAL];
	sampleEditorGridOn  = j[CONF_KEY_SAMPLE_EDITOR_GRID_ON];
	pianoRollY = j[CONF_KEY_PIANO_ROLL_Y];
	pianoRollH = j[CONF_KEY_PIANO_ROLL_H];
	sampleActionEditorH = j[CONF_KEY_SAMPLE_ACTION_EDITOR_H];
	velocityEditorH = j[CONF_KEY_VELOCITY_EDITOR_H];
	envelopeEditorH = j[CONF_KEY_ENVELOPE_EDITOR_H];
	pluginListX = j[CONF_KEY_PLUGIN_LIST_X];
	pluginListY = j[CONF_KEY_PLUGIN_LIST_Y];
	configX = j[CONF_KEY_CONFIG_X];
	configY = j[CONF_KEY_CONFIG_Y];
	bpmX = j[CONF_KEY_BPM_X];
	bpmY = j[CONF_KEY_BPM_Y];
	beatsX = j[CONF_KEY_BEATS_X];
	beatsY = j[CONF_KEY_BEATS_Y];
	aboutX = j[CONF_KEY_ABOUT_X];
	aboutY = j[CONF_KEY_ABOUT_Y];
	nameX = j[CONF_KEY_NAME_X];
	nameY = j[CONF_KEY_NAME_Y];
	midiInputX = j[CONF_KEY_MIDI_INPUT_X];
	midiInputY = j[CONF_KEY_MIDI_INPUT_Y];
	midiInputW = j[CONF_KEY_MIDI_INPUT_W];
	midiInputH = j[CONF_KEY_MIDI_INPUT_H];
	recTriggerMode  = j[CONF_KEY_REC_TRIGGER_MODE];
	recTriggerLevel = j[CONF_KEY_REC_TRIGGER_LEVEL];

#ifdef WITH_VST

	pluginChooserX   = j[CONF_KEY_PLUGIN_CHOOSER_X];
	pluginChooserY   = j[CONF_KEY_PLUGIN_CHOOSER_Y];
	pluginChooserW   = j[CONF_KEY_PLUGIN_CHOOSER_W];
	pluginChooserH   = j[CONF_KEY_PLUGIN_CHOOSER_H];
	pluginSortMethod = j[CONF_KEY_PLUGIN_SORT_METHOD];

#endif

	sanitize();

	return true;
}


/* -------------------------------------------------------------------------- */


bool write()
{
	if (!createConfigFolder_())
		return false;

	nl::json j;

	j[CONF_KEY_HEADER] =                    header;
	j[CONF_KEY_LOG_MODE] =                  logMode;
	j[CONF_KEY_SOUND_SYSTEM] =              soundSystem;
	j[CONF_KEY_SOUND_DEVICE_OUT] =          soundDeviceOut;
	j[CONF_KEY_SOUND_DEVICE_IN] =           soundDeviceIn;
	j[CONF_KEY_CHANNELS_OUT] =              channelsOut;
	j[CONF_KEY_CHANNELS_IN] =               channelsIn;
	j[CONF_KEY_SAMPLERATE] =                samplerate;
	j[CONF_KEY_BUFFER_SIZE] =               buffersize;
	j[CONF_KEY_LIMIT_OUTPUT] =              limitOutput;
	j[CONF_KEY_RESAMPLE_QUALITY] =          rsmpQuality;
	j[CONF_KEY_MIDI_SYSTEM] =               midiSystem;
	j[CONF_KEY_MIDI_PORT_OUT] =             midiPortOut;
	j[CONF_KEY_MIDI_PORT_IN] =              midiPortIn;
	j[CONF_KEY_MIDIMAP_PATH] =              midiMapPath;
	j[CONF_KEY_LAST_MIDIMAP] =              lastFileMap;
	j[CONF_KEY_MIDI_SYNC] =                 midiSync;
	j[CONF_KEY_MIDI_TC_FPS] =               midiTCfps;
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
	j[CONF_KEY_RECS_STOP_ON_CHAN_HALT] =    recsStopOnChanHalt;
	j[CONF_KEY_CHANS_STOP_ON_SEQ_HALT] =    chansStopOnSeqHalt;
	j[CONF_KEY_TREAT_RECS_AS_LOOPS] =       treatRecsAsLoops;
	j[CONF_KEY_INPUT_MONITOR_DEFAULT_ON] =  inputMonitorDefaultOn;
	j[CONF_KEY_PLUGINS_PATH] =              pluginPath;
	j[CONF_KEY_PATCHES_PATH] =              patchPath;
	j[CONF_KEY_SAMPLES_PATH] =              samplePath;
	j[CONF_KEY_MAIN_WINDOW_X] =             mainWindowX;
	j[CONF_KEY_MAIN_WINDOW_Y] =             mainWindowY;
	j[CONF_KEY_MAIN_WINDOW_W] =             mainWindowW;
	j[CONF_KEY_MAIN_WINDOW_H] =             mainWindowH;
	j[CONF_KEY_BROWSER_X] =                 browserX;
	j[CONF_KEY_BROWSER_Y] =                 browserY;
	j[CONF_KEY_BROWSER_W] =                 browserW;
	j[CONF_KEY_BROWSER_H] =                 browserH;
	j[CONF_KEY_BROWSER_POSITION] =          browserPosition;
	j[CONF_KEY_BROWSER_LAST_PATH] =         browserLastPath;
	j[CONF_KEY_BROWSER_LAST_VALUE] =        browserLastValue;
	j[CONF_KEY_ACTION_EDITOR_X] =           actionEditorX;
	j[CONF_KEY_ACTION_EDITOR_Y] =           actionEditorY;
	j[CONF_KEY_ACTION_EDITOR_W] =           actionEditorW;
	j[CONF_KEY_ACTION_EDITOR_H] =           actionEditorH;
	j[CONF_KEY_ACTION_EDITOR_ZOOM] =        actionEditorZoom;
	j[CONF_KEY_ACTION_EDITOR_GRID_VAL] =    actionEditorGridVal;
	j[CONF_KEY_ACTION_EDITOR_GRID_ON] =     actionEditorGridOn;
	j[CONF_KEY_SAMPLE_EDITOR_X] =           sampleEditorX;
	j[CONF_KEY_SAMPLE_EDITOR_Y] =           sampleEditorY;
	j[CONF_KEY_SAMPLE_EDITOR_W] =           sampleEditorW;
	j[CONF_KEY_SAMPLE_EDITOR_H] =           sampleEditorH;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_VAL] =    sampleEditorGridVal;
	j[CONF_KEY_SAMPLE_EDITOR_GRID_ON] =     sampleEditorGridOn;
	j[CONF_KEY_PIANO_ROLL_Y] =              pianoRollY;
	j[CONF_KEY_PIANO_ROLL_H] =              pianoRollH;
	j[CONF_KEY_SAMPLE_ACTION_EDITOR_H]    = sampleActionEditorH;
	j[CONF_KEY_VELOCITY_EDITOR_H]         = velocityEditorH;
	j[CONF_KEY_ENVELOPE_EDITOR_H]         = envelopeEditorH;
	j[CONF_KEY_PLUGIN_LIST_X]             = pluginListX;
	j[CONF_KEY_PLUGIN_LIST_Y]             = pluginListY;
	j[CONF_KEY_CONFIG_X]                  = configX;
	j[CONF_KEY_CONFIG_Y]                  = configY;
	j[CONF_KEY_BPM_X]                     = bpmX;
	j[CONF_KEY_BPM_Y]                     = bpmY;
	j[CONF_KEY_BEATS_X]                   = beatsX;
	j[CONF_KEY_BEATS_Y]                   = beatsY;
	j[CONF_KEY_ABOUT_X]                   = aboutX;
	j[CONF_KEY_ABOUT_Y]                   = aboutY;
	j[CONF_KEY_NAME_X]                    = nameX;
	j[CONF_KEY_NAME_Y]                    = nameY;
	j[CONF_KEY_MIDI_INPUT_X]              = midiInputX;
	j[CONF_KEY_MIDI_INPUT_Y]              = midiInputY;
	j[CONF_KEY_MIDI_INPUT_W]              = midiInputW;
	j[CONF_KEY_MIDI_INPUT_H]              = midiInputH;
	j[CONF_KEY_REC_TRIGGER_MODE]          = recTriggerMode;
	j[CONF_KEY_REC_TRIGGER_LEVEL]         = recTriggerLevel;

#ifdef WITH_VST

	j[CONF_KEY_PLUGIN_CHOOSER_X]   = pluginChooserX;
	j[CONF_KEY_PLUGIN_CHOOSER_Y]   = pluginChooserY;
	j[CONF_KEY_PLUGIN_CHOOSER_W]   = pluginChooserW;
	j[CONF_KEY_PLUGIN_CHOOSER_H]   = pluginChooserH;
	j[CONF_KEY_PLUGIN_SORT_METHOD] = pluginSortMethod;

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