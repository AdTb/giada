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


#ifndef G_WAVE_FX_H
#define G_WAVE_FX_H


namespace giada {
namespace m 
{
class Wave;

namespace wfx
{
static const int FADE_IN  = 0;
static const int FADE_OUT = 1;
static const int SMOOTH_SIZE = 32;

/* monoToStereo
Converts a 1-channel Wave to a 2-channels wave. It works on a free Wave object,
not yet added to the RCUList. */

int monoToStereo(Wave& w);

/* normalizeHard
Normalizes the wave in range a-b by altering values in memory. */

void normalizeHard(ID waveId, int a, int b);

void silence(ID waveId, int a, int b);
void cut(ID waveId, int a, int b);
void trim(ID waveId, int a, int b);

/* paste
Pastes Wave 'src' into Wave at 'waveIndex', starting from frame 'a'. */

void paste(const Wave& src, ID waveId, int a);

/* fade
Fades in or fades out selection. Fade In = type 0, Fade Out = type 1 */

void fade(ID waveId, int a, int b, int type);

/* smooth
Smooth edges of selection. */

void smooth(ID waveId, int a, int b);

/* reverse
Flips Wave's data. */

void reverse(ID waveId, int a, int b);

void shift(ID waveId, int offset);
}}}; // giada::m::wfx::


#endif
