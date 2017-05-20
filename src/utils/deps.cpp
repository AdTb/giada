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


#include <rtmidi/RtMidi.h>
#include <sndfile.h>
#include "../deps/rtaudio-mod/RtAudio.h"
#include "deps.h"


using std::string;


namespace giada {
namespace u     {
namespace deps  
{
string getLibsndfileVersion()
{
  char buffer[128];
  sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof(buffer));
  return string(buffer);
}


/* -------------------------------------------------------------------------- */


string getRtAudioVersion()
{
  return RtAudio::getVersion();
}


/* -------------------------------------------------------------------------- */


string getRtMidiVersion()
{
  return RtMidi::getVersion();
}

}}};  // giada::u::deps::