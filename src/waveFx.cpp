/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * waveFx
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#include "waveFx.h"


extern Mixer G_Mixer;


float wfx_normalizeSoft(Wave *w) {
	float peak = 0.0f;
	float abs  = 0.0f;
	for (unsigned i=0; i<w->size; i++) { // i++: both L and R samples
		abs = fabs(w->data[i]);
		if (abs > peak)
			peak = abs;
	}

	/* peak == 0.0f: don't normalize the silence
	 * peak > 1.0f: don't reduce the amplitude, just leave it alone */

	if (peak == 0.0f || peak > 1.0f)
		return 1.0f;

	return 1.0f / peak;
}


/* ------------------------------------------------------------------ */


bool wfx_monoToStereo(Wave *w) {

	unsigned newSize = w->size * 2;
	float *dataNew = (float *) malloc(newSize * sizeof(float));
	if (dataNew == NULL) {
		printf("[wfx] unable to allocate memory for mono>stereo conversion\n");
		return 0;
	}

	for (unsigned i=0, j=0; i<w->size; i++) {
		dataNew[j]   = w->data[i];
		dataNew[j+1] = w->data[i];
		j+=2;
	}

	free(w->data);
	w->data = dataNew;
	w->size = newSize;
	w->inHeader.frames *= 2;
	w->inHeader.channels = 2;

	return 1;
}


/* ------------------------------------------------------------------ */


void wfx_silence(Wave *w, unsigned a, unsigned b) {

	/* stereo values */
	a = a * 2;
	b = b * 2;

	printf("[wfx] silencing from %d to %d\n", a, b);

	for (unsigned i=a; i<b; i+=2) {
		w->data[i]   = 0.0f;
		w->data[i+1] = 0.0f;
	}

	w->isEdited = true;

	return;
}


/* ------------------------------------------------------------------ */


int wfx_cut(int ch, unsigned a, unsigned b) {

	a = a * 2;
	b = b * 2;

	if (a < 0) a = 0;
	if (b > G_Mixer.chan[ch]->size) b = G_Mixer.chan[ch]->size;

	/* create a new temp wave and copy there the original one, skipping
	 * the a-b range */

	unsigned newSize = G_Mixer.chan[ch]->size-(b-a);
	float *temp = (float *) malloc(newSize * sizeof(float));
	if (temp == NULL) {
		puts("[wfx] unable to allocate memory for cutting");
		return 0;
	}

	printf("[wfx] cutting from %d to %d, new size=%d (video=%d)\n", a, b, newSize, newSize/2);

	for (unsigned i=0, k=0; i<G_Mixer.chan[ch]->size; i++) {
		if (i < a || i >= b) {		               // left margin always included, in order to keep
			temp[k] = G_Mixer.chan[ch]->data[i];   // the stereo pair
			k++;
		}
	}

	free(G_Mixer.chan[ch]->data);
	G_Mixer.chan[ch]->data = temp;
	G_Mixer.chan[ch]->size = newSize;
	G_Mixer.chan[ch]->inHeader.frames -= b-a;
	G_Mixer.chan[ch]->isEdited = true;

	puts("[wfx] cutting done");

	return 1;
}


/* ------------------------------------------------------------------ */


int wfx_trim(int ch, unsigned a, unsigned b) {

	a = a * 2;
	b = b * 2;

	if (a < 0) a = 0;
	if (b > G_Mixer.chan[ch]->size) b = G_Mixer.chan[ch]->size;

	unsigned newSize = b - a;
	float *temp = (float *) malloc(newSize * sizeof(float));
	if (temp == NULL) {
		puts("[wfx] unable to allocate memory for trimming");
		return 0;
	}

	printf("[wfx] trimming from %d to %d (area = %d)\n", a, b, b-a);

	for (unsigned i=a, k=0; i<b; i++, k++)
		temp[k] = G_Mixer.chan[ch]->data[i];

	free(G_Mixer.chan[ch]->data);
	G_Mixer.chan[ch]->data = temp;
	G_Mixer.chan[ch]->size = newSize;
	G_Mixer.chan[ch]->inHeader.frames = b-a;
 	G_Mixer.chan[ch]->isEdited = true;

	return 1;
}
