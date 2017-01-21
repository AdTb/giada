/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * wave
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


#ifndef WAVE_H
#define WAVE_H


#include <samplerate.h>
#include <sndfile.h>
#include <string>


using std::string;


class Wave
{
private:

	SNDFILE   *fileIn;
	SNDFILE   *fileOut;
	SF_INFO    inHeader;
	SF_INFO    outHeader;


public:

	Wave();
	~Wave();
	Wave(const Wave &other);

	string pathfile; // full path + sample name
	string name;			// sample name (changeable)

	float     *data;
	int        size;			  // wave size (size in stereo: size / 2)
	bool       isLogical;   // memory only (a take)
	bool       isEdited;    // edited via editor

	inline int  rate    () { return inHeader.samplerate; }
	inline int  channels() { return inHeader.channels; }
	inline int  frames  () { return inHeader.frames; }
	inline void rate    (int v) { inHeader.samplerate = v; }
	inline void channels(int v) { inHeader.channels = v; }
	inline void frames  (int v) { inHeader.frames = v; }

	string basename (bool ext=false) const;
	string extension() const;

	void updateName(const char *n);
	int  open      (const char *f);
	int  readData  ();
	int	 writeData (const char *f);
	void clear     ();

	/* allocEmpty
	 * alloc an empty waveform. */

	int allocEmpty(unsigned size, unsigned samplerate);

	/* resample
	 * simple algorithm for one-shot resampling. */

	int resample(int quality, int newRate);
};

#endif
