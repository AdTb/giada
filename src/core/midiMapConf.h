/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiMapConf
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


#ifndef __MIDIMAPCONF_H__
#define __MIDIMAPCONF_H__


#include <limits.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "dataStorageIni.h"
#include "dataStorageJson.h"
#include "../utils/fs.h"
#if defined(__APPLE__)
#include <pwd.h>
#endif


class MidiMapConf : public DataStorageIni, public DataStorageJson
{
public:

	struct message_t
  {
    int         channel;
    std::string valueStr;
		int         offset;
		uint32_t    value;
  };

	std::string brand;
  std::string device;
	std::vector<message_t> initCommands;
	message_t muteOn;
	message_t muteOff;
	message_t soloOn;
	message_t soloOff;
	message_t waiting;
	message_t playing;
	message_t stopping;
	message_t stopped;

	/* midimapsPath
	 * path of midimap files, different between OSes. */

	std::string midimapsPath;

	/* maps
	 * Maps are the available .giadamap files. Each element of the std::vector
	 * represents a .giadamap filename. */

	std::vector<std::string> maps;

	/* init
	Parse the midi maps folders and find the available maps. */

	void init();

	/* setDefault
	Set default values in case no maps are available/choosen. */

	void setDefault();

	/* read
	Read a midi map from file 'file'. */

	int read(const std::string &file);

	/* --- DEPRECATED STUFF --------------------------------------------------- */
	/* --- DEPRECATED STUFF --------------------------------------------------- */
	/* --- DEPRECATED STUFF --------------------------------------------------- */

	static const int MAX_INIT_COMMANDS = 32;
	static const int MAX_MIDI_BYTES = 4;
	static const int MAX_MIDI_NIBBLES = 8;

	/* init_*
	 * init_commands. These messages are sent to the physical device as a wake up
	 * signal. */

	int      init_channels[MAX_INIT_COMMANDS];
	uint32_t init_messages[MAX_INIT_COMMANDS];

	/* events
	 * [event]Channel: the MIDI output channel to send the event to
	 * [event]notePos: the byte where the note is stored ('nn' placeholder)
	 * [event]offset:  the note offset (i.e. of 'nn' placeholder) */

	int      muteOnChan;
	int      muteOnOffset;
	uint32_t muteOnMsg;

	int      muteOffChan;
	int      muteOffOffset;
	uint32_t muteOffMsg;

	int      soloOnChan;
	int      soloOnOffset;
	uint32_t soloOnMsg;

	int      soloOffChan;
	int      soloOffOffset;
	uint32_t soloOffMsg;

	int      waitingChan;
	int      waitingOffset;
	uint32_t waitingMsg;

	int      playingChan;
	int      playingOffset;
	uint32_t playingMsg;

	int      stoppingChan;
	int      stoppingOffset;
	uint32_t stoppingMsg;

	int      stoppedChan;
	int      stoppedOffset;
	uint32_t stoppedMsg;

	/* setDefault
	Set default values in case no maps are available/choosen. */

	void setDefault_DEPR_();

	/* readMap
	Read a midi map from file 'file'. */

	int readMap_DEPR_(std::string file);

private:

	bool readInitCommands(json_t *jContainer);

	bool readCommand(json_t *jContainer, message_t *msg, const std::string &key);

	void parse(message_t *message);

	/* --- DEPRECATED STUFF --------------------------------------------------- */
	/* --- DEPRECATED STUFF --------------------------------------------------- */
	/* --- DEPRECATED STUFF --------------------------------------------------- */

	void close_DEPR_();
	void parse_DEPR_(std::string key, int *chan, uint32_t *msg, int *offset);
};

#endif
