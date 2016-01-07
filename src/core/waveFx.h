/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * waveFx
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#ifndef WAVEFX_H
#define WAVEFX_H


/* normalizeSoft
 * normalize the wave by returning the dB value for the boost volume. It
 * doesn't deal with data in memory. */

float wfx_normalizeSoft(class Wave *w);

bool wfx_monoToStereo(class Wave *w);

void wfx_silence(class Wave *w, int a, int b);

int wfx_cut(class Wave *w, int a, int b);

int wfx_trim(class Wave *w, int a, int b);

/* fade
 * fade in or fade out selection. Fade In = type 0, Fade Out = type 1 */

void wfx_fade(class Wave *w, int a, int b, int type);

/* smooth
 * smooth edges of selection. */

void wfx_smooth(class Wave *w, int a, int b);


#endif
