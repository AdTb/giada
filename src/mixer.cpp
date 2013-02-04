/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixer
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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


#include "mixer.h"
#include "init.h"
#include "wave.h"
#include "gui_utils.h"
#include "recorder.h"
#include "pluginHost.h"


extern Mixer 			 G_Mixer;
extern unsigned 	 G_beats;
extern Patch		 	 G_Patch;
extern Conf				 G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Mixer::Mixer() 	{}
Mixer::~Mixer() {}


#define TICKSIZE 38


float Mixer::tock[TICKSIZE] = {
	 0.059033,  0.117240,  0.173807,  0.227943,  0.278890,  0.325936,
	 0.368423,  0.405755,  0.437413,  0.462951,  0.482013,  0.494333,
	 0.499738,  0.498153,  0.489598,  0.474195,  0.452159,  0.423798,
	 0.389509,  0.349771,  0.289883,  0.230617,  0.173194,  0.118739,
	 0.068260,  0.022631, -0.017423, -0.051339,	-0.078721, -0.099345,
	-0.113163, -0.120295, -0.121028, -0.115804, -0.105209, -0.089954,
	-0.070862, -0.048844
};


float Mixer::tick[TICKSIZE] = {
	  0.175860,  0.341914,  0.488904,  0.608633,  0.694426,  0.741500,
	  0.747229,  0.711293,	0.635697,  0.524656,  0.384362,  0.222636,
	  0.048496, -0.128348, -0.298035, -0.451105, -0.579021, -0.674653,
	 -0.732667, -0.749830, -0.688924, -0.594091, -0.474481, -0.340160,
	 -0.201360, -0.067752,  0.052194,  0.151746,  0.226280,  0.273493,
	  0.293425,  0.288307,  0.262252,  0.220811,  0.170435,  0.117887,
	  0.069639,  0.031320
};


/* ------------------------------------------------------------------ */


void Mixer::init() {

	vChanInput = NULL;

	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {
		chan[i]          = NULL;
		vChan[i] 				 = NULL;
		chanTracker[i]   = 0;
		chanStatus[i]    = STATUS_EMPTY;
		chanStart[i]     = 0;
		chanEnd[i]       = 0;
		chanMute[i]      = false;
		chanMute_i[i]    = false;
		chanMode[i]      = DEFAULT_CHANMODE;
		chanVolume[i]    = DEFAULT_VOL;
		chanPitch[i]     = gDEFAULT_PITCH;
		chanBoost[i]     = 1.0f;
		chanPanLeft[i]   = 1.0f;
		chanPanRight[i]  = 1.0f;
		chanQWait[i]	   = false;
		chanRecStatus[i] = REC_STOPPED;
		chanFadein[i]    = 1.0f;
		fadeoutOn[i]     = false;
		fadeoutVol[i]    = 1.0f;
		fadeoutStep[i]   = DEFAULT_FADEOUT_STEP;

		/* virtual channel malloc. We use kernelAudio::realBufsize, the real
		 * buffer size from the soundcard. G_Conf.bufferSize may be wrong or
		 * useless, especially when JACK is running. */

		vChan[i] = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
	}

	quanto      = 1;
	docross     = false;
	rewindWait  = false;
	running     = false;
	waitRec     = 0;
	actualFrame = 0;
	bpm 		    = DEFAULT_BPM;
	bars		    = DEFAULT_BARS;
	beats		    = DEFAULT_BEATS;
	quantize    = DEFAULT_QUANTIZE;
	metronome   = false;

	tickTracker = 0;
	tockTracker = 0;
	tickPlay    = false;
	tockPlay    = false;

	outVol       = DEFAULT_OUT_VOL;
	inVol        = DEFAULT_IN_VOL;
	peakOut      = 0.0f;
	peakIn	     = 0.0f;
	chanInput    = -1;
	inputTracker = 0;

	/* alloc virtual input channels. vChanInput malloc is done in
	 * updateFrameBars, because of its variable size */

	vChanInput   = NULL;
	vChanInToOut = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));

	pthread_mutex_init(&mutex_recs, NULL);
	pthread_mutex_init(&mutex_plugins, NULL);

	updateFrameBars();
	rewind();
}


/* ------------------------------------------------------------------ */


void Mixer::loadWave(Wave *w, int ch) {
	chan[ch]				  = w;
	chanStatus[ch] 	  = STATUS_OFF;
	chanStart[ch]     = 0;
	chanStartTrue[ch] = 0;
	chanEnd[ch]			  = chan[ch]->size;
	chanEndTrue[ch]	  = chan[ch]->size;
}


/* ------------------------------------------------------------------ */


void Mixer::freeWave(int ch) {
	chanStatus[ch]  = STATUS_EMPTY;
	chanTracker[ch] = 0;
	chanBoost[ch]   = 1.0f;
}


/* ------------------------------------------------------------------ */


void Mixer::chanStop(int ch) {
	chanStatus[ch] = STATUS_OFF;
	chanReset(ch);
}


/* ------------------------------------------------------------------ */


void Mixer::chanReset(int ch)	{
	chanTracker[ch] = chanStart[ch];
	chanMute_i [ch] = false;
}


/* ------------------------------------------------------------------ */


void Mixer::fadein(int ch, bool internal) {

	/* remove mute before fading in */

	if (internal) chanMute_i[ch] = false;
	else          chanMute[ch]   = false;
	chanFadein[ch] = 0.0f;
}


/* ------------------------------------------------------------------ */


void Mixer::fadeout(int ch, int actionPostFadeout) {
	calcFadeoutStep(ch);
	fadeoutOn[ch]      = true;
	fadeoutVol[ch]     = 1.0f;
	fadeoutType[ch]    = FADEOUT;
	fadeoutEnd[ch]		 = actionPostFadeout;
}


/* ------------------------------------------------------------------ */


void Mixer::xfade(int ch) {
	calcFadeoutStep(ch);
	fadeoutOn[ch]      = true;
	fadeoutVol[ch]     = 1.0f;
	fadeoutTracker[ch] = chanTracker[ch];
	fadeoutType[ch]    = XFADE;
	chanReset(ch);
}


/* ------------------------------------------------------------------ */


int Mixer::getChanPos(int ch)	{
	if (chanStatus[ch] & ~(STATUS_EMPTY | STATUS_MISSING | STATUS_OFF))  // if chanStatus[ch] is not (...)
		return chanTracker[ch] - chanStart[ch];
	else
		return -1;
}


/* ------------------------------------------------------------------ */


int Mixer::masterPlay(
	void *out_buf, void *in_buf, unsigned n_frames,
	double streamTime, RtAudioStreamStatus status, void *userData) {
	return G_Mixer.__masterPlay(out_buf, in_buf, n_frames);
}


/* ------------------------------------------------------------------ */


int Mixer::__masterPlay(void *out_buf, void *in_buf, unsigned bufferFrames) {

	float *buffer  = ((float *) out_buf);
	float *inBuf   = ((float *) in_buf);
	bufferFrames  *= 2;     // stereo
	peakOut        = 0.0f;  // reset peak calculator
	peakIn         = 0.0f;  // reset peak calculator

	/* always clean each buffer */

	memset(buffer, 0, sizeof(float) * bufferFrames);         // out
	memset(vChanInToOut, 0, sizeof(float) * bufferFrames);   // inToOut vChan
	for (unsigned i=0; i<MAX_NUM_CHAN; i++)
		memset(vChan[i], 0, sizeof(float) * bufferFrames);     // vchans

	for (unsigned j=0; j<bufferFrames; j+=2) {

		if (kernelAudio::inputEnabled) {

			/* input peak calculation (left chan only so far). */

			if (inBuf[j] * inVol > peakIn)
				peakIn = inBuf[j] * inVol;

			/* "hear what you're playing" - process, copy and paste the input buffer
			 * onto the output buffer */

			if (inToOut) {
				vChanInToOut[j]   = inBuf[j]   * inVol;
				vChanInToOut[j+1] = inBuf[j+1] * inVol;
			}
		}

		/* operations to do if the sequencer is running:
		 * - compute quantizer
		 * - time check for LOOP_REPEAT
		 * - reset loops at beat 0
		 * - read recorded actions
		 * - reset actualFrame */

		if (running) {

			/* line in recording */

			if (chanInput != -1 && kernelAudio::inputEnabled) {

				/* delay comp: wait until waitRec reaches delayComp. WaitRec
				 * returns to 0 in mixerHandler, as soon as the recording ends */

				if (waitRec < G_Conf.delayComp)
					waitRec += 2;
				else {
					vChanInput[inputTracker]   += inBuf[j]   * inVol;
					vChanInput[inputTracker+1] += inBuf[j+1] * inVol;
					inputTracker += 2;
					if (inputTracker >= totalFrames)
						inputTracker = 0;
				}
			}

			/* quantizer computations */

			if (quantize > 0 && quanto > 0) {

				bool isQuanto = actualFrame % (quanto) == 0 ? true : false;

				/* rewind quantization */

				if (isQuanto && rewindWait) {
					rewindWait = false;
					rewind();
				}

				for (unsigned k=0; k<MAX_NUM_CHAN; k++) {
					if (chan[k] == NULL)
						continue;
					if (isQuanto && (chanMode[k] & SINGLE_ANY) && chanQWait[k] == true)	{

						/* no fadeout if the sample starts for the first time (from a STATUS_OFF), it would
						 * be meaningless. */

						if (chanStatus[k] == STATUS_OFF) {
							chanStatus[k] = STATUS_PLAY;
							chanQWait[k]  = false;
						}
						else
							xfade(k);

						/* this is the moment in which we record the keypress, if the quantizer is on.
						 * SINGLE_PRESS needs overdub */

						if (recorder::canRec(k)) {
							if (chanMode[k] == SINGLE_PRESS)
								recorder::startOverdub(k, ACTION_KEYS, actualFrame);
							else
								recorder::rec(k, ACTION_KEYPRESS, actualFrame);
						}
					}
				}
			}

			/* reset LOOP_REPEAT, if a bar has passed */

			if (actualFrame % framesPerBar == 0 && actualFrame != 0) {
				if (metronome)
					tickPlay = true;

				for (unsigned k=0; k<MAX_NUM_CHAN; k++) {
					if (chan[k] == NULL)
						continue;
					if (chanMode[k] == LOOP_REPEAT && chanStatus[k] == STATUS_PLAY)
						xfade(k);
				}
			}


			/* reset sample on beat 0 */

			if (actualFrame == 0) {
				for (unsigned k=0; k<MAX_NUM_CHAN; k++) {

					if (chan[k] == NULL)
						continue;

					if (chanMode[k] & (LOOP_ONCE | LOOP_BASIC)) { // LOOP_REPEAT already done

						/* do a crossfade if the sample is playing. Regular chanReset
						 * instead if it's muted, otherwise a click occurs */

						if (chanStatus[k] == STATUS_PLAY) {
							if (chanMute[k] || chanMute_i[k])
								chanReset(k);
							else
								xfade(k);
						}
						else
						if (chanStatus[k] == STATUS_ENDING)
							chanStop(k);
					}

					if (chanStatus[k] == STATUS_WAIT)
						chanStatus[k] = STATUS_PLAY;

					if (chanRecStatus[k] == REC_ENDING) {
						chanRecStatus[k] = REC_STOPPED;
						recorder::disableRead(k);    // rec stop
					}
					else
					if (chanRecStatus[k] == REC_WAITING) {
						chanRecStatus[k] = REC_READING;
						recorder::enableRead(k);     // rec start
					}
				}
			}

			/* reading all actions recorded */

			pthread_mutex_lock(&mutex_recs);
			for (unsigned y=0; y<recorder::frames.size; y++) {
				if (recorder::frames.at(y) == actualFrame) {
					for (unsigned z=0; z<recorder::global.at(y).size; z++) {
						int chan = recorder::global.at(y).at(z)->chan;
						if (recorder::chanActive[chan] == false)
							continue;
						switch (recorder::global.at(y).at(z)->type) {
							case ACTION_KEYPRESS:
								if (chanMode[chan] & SINGLE_ANY) {
									mh_startChan(chan, false);
									break;
								}
							case ACTION_KEYREL:
								if (chanMode[chan] & SINGLE_ANY) {
									mh_stopChan(chan);
									break;
								}
							case ACTION_KILLCHAN:
								if (chanMode[chan] & SINGLE_ANY) {
									mh_killChan(chan);
									break;
								}
							case ACTION_MUTEON:
								mh_muteChan(chan, true);   // internal mute
								break;
							case ACTION_MUTEOFF:
								mh_unmuteChan(chan, true); // internal mute
								break;
						}
					}
					break;
				}
			}
			pthread_mutex_unlock(&mutex_recs);

			actualFrame += 2;

			/* if actualFrame > totalFrames the sequencer returns to frame 0,
			 * beat 0. This must be the last operation. */

			if (actualFrame > totalFrames) {
				actualFrame = 0;
				G_beats     = 0;
			}
			else if (actualFrame % framesPerBeat == 0 && actualFrame > 0) {
				G_beats++;

				/* avoid tick and tock to overlap when a new bar has passed (which
				 * is also a beat) */

				if (metronome && !tickPlay)
					tockPlay = true;
			}
		} // if (running)


		/* sum channels  */

		for (unsigned k=0; k<MAX_NUM_CHAN; k++) {
			if (chan[k] == NULL)
				continue;

			if (chanStatus[k] & (STATUS_PLAY | STATUS_ENDING)) {

				if (chanTracker[k] <= chanEnd[k]) {

					/* ctrakcer is chanTracker, pitch affected */

					unsigned ctracker = chanTracker[k]*chanPitch[k];

					/* fade in */

					if (chanFadein[k] <= 1.0f)
						chanFadein[k] += 0.01f;		/// FIXME - remove the hardcoded value

					/* fadeout process (both fadeout and xfade) */

					if (fadeoutOn[k]) {
						if (fadeoutVol[k] >= 0.0f) { // fadeout ongoing

							if (fadeoutType[k] == XFADE) {

								/* ftracker is fadeoutTracker affected by pitch */

								unsigned ftracker = fadeoutTracker[k]*chanPitch[k];

								vChan[k][j]   += chan[k]->data[ftracker]   * chanVolume[k] * fadeoutVol[k] * chanBoost[k] * chanPanLeft[k];
								vChan[k][j+1] += chan[k]->data[ftracker+1] * chanVolume[k] * fadeoutVol[k] * chanBoost[k] * chanPanRight[k];

								vChan[k][j]   += chan[k]->data[ctracker]   * chanVolume[k] * chanBoost[k] * chanPanLeft[k];
								vChan[k][j+1] += chan[k]->data[ctracker+1] * chanVolume[k] * chanBoost[k] * chanPanRight[k];
							}
							else { // FADEOUT
								vChan[k][j]   += chan[k]->data[ctracker]   * chanVolume[k] * fadeoutVol[k] * chanBoost[k] * chanPanLeft[k];
								vChan[k][j+1] += chan[k]->data[ctracker+1] * chanVolume[k] * fadeoutVol[k] * chanBoost[k] * chanPanRight[k];
							}

							fadeoutVol[k]     -= fadeoutStep[k];
							fadeoutTracker[k] += 2;
						}
						else {  // fadeout end
							fadeoutOn[k]  = false;
							fadeoutVol[k] = 1.0f;

							/* QWait ends with the end of the xfade */

							if (fadeoutType[k] == XFADE) {
								chanQWait[k] = false;
							}
							else {
								if (fadeoutEnd[k] == DO_MUTE)
									chanMute[k] = true;
								else
								if (fadeoutEnd[k] == DO_MUTE_I)
									chanMute_i[k] = true;
								else              // DO_STOP
									chanStop(k);
							}

							/* we must append another frame in the buffer when the fadeout
							 * ends: there's a gap here which would clip otherwise */

							vChan[k][j]   = vChan[k][j-2];
							vChan[k][j+1] = vChan[k][j-1];
						}
					}  // no fadeout to do
					else {
						if (!chanMute[k] && !chanMute_i[k]) {
							vChan[k][j]   += chan[k]->data[ctracker]   * chanVolume[k] * chanFadein[k] * chanBoost[k] * chanPanLeft[k];
							vChan[k][j+1] += chan[k]->data[ctracker+1] * chanVolume[k] * chanFadein[k] * chanBoost[k] * chanPanRight[k];
						}
					}

					chanTracker[k] += 2;

					/* check for end of samples */

					if (chanTracker[k] >= chanEnd[k]) {

						chanReset(k);

						if (chanMode[k] & SINGLE_ANY)
							chanStatus[k] = STATUS_OFF;

						/* stop loops when the seq is off */

						if ((chanMode[k] & LOOP_ANY) && !running)
							chanStatus[k] = STATUS_OFF;

						/* temporary stop LOOP_ONCE not in ENDING status, otherwise they
						 * would return in wait, losing the ENDING status */

						if (chanMode[k] == LOOP_ONCE && chanStatus[k] != STATUS_ENDING)
							chanStatus[k] = STATUS_WAIT;
					}
				}
			}
		} // end loop K

		/* metronome. FIXME - move this one after the peak meter calculation */

		if (tockPlay) {
			buffer[j]   += tock[tockTracker];
			buffer[j+1] += tock[tockTracker];
			tockTracker++;
			if (tockTracker >= TICKSIZE-1) {
				tockPlay    = false;
				tockTracker = 0;
			}
		}
		if (tickPlay) {
			buffer[j]   += tick[tickTracker];
			buffer[j+1] += tick[tickTracker];
			tickTracker++;
			if (tickTracker >= TICKSIZE-1) {
				tickPlay    = false;
				tickTracker = 0;
			}
		}
	} // end loop J

	/* final loop: sum virtual channels */

	for (unsigned k=0; k<MAX_NUM_CHAN; k++) {
		if (chan[k] != NULL) {
#ifdef WITH_VST
			pthread_mutex_lock(&mutex_plugins);
			G_PluginHost.processStack(vChan[k], PluginHost::CHANNEL, k);
			pthread_mutex_unlock(&mutex_plugins);
#endif
			for (unsigned j=0; j<bufferFrames; j+=2) {
				buffer[j]   += vChan[k][j];
				buffer[j+1] += vChan[k][j+1];
			}
		}
	}

	/* processing fxs master in & out, if any. */

#ifdef WITH_VST
	pthread_mutex_lock(&mutex_plugins);
	G_PluginHost.processStack(buffer, PluginHost::MASTER_OUT);
	G_PluginHost.processStack(vChanInToOut, PluginHost::MASTER_IN);
	pthread_mutex_unlock(&mutex_plugins);
#endif

	/* post processing fx of master + peak calculation. */

	for (unsigned j=0; j<bufferFrames; j+=2) {

		/* merging vChanInToOut, if enabled */

		if (inToOut) {
			buffer[j]   += vChanInToOut[j];
			buffer[j+1] += vChanInToOut[j+1];
		}

		buffer[j]   *= outVol;
		buffer[j+1] *= outVol;

		/* computes the peak for the left channel (so far). */

		if (buffer[j] > peakOut)
			peakOut = buffer[j];

		if (G_Conf.limitOutput) {
			if (buffer[j] > 1.0f)
				buffer[j] = 1.0f;
			else if (buffer[j] < -1.0f)
				buffer[j] = -1.0f;

			if (buffer[j+1] > 1.0f)
				buffer[j+1] = 1.0f;
			else if (buffer[j+1] < -1.0f)
				buffer[j+1] = -1.0f;
		}
	}

	return 0;
}


/* ------------------------------------------------------------------ */


void Mixer::updateFrameBars() {

	/* seconds ....... total time of play (in seconds) of the whole
	 *                 sequencer. 60 / bpm == how many seconds lasts one bpm
	 * totalFrames ... number of frames in the whole sequencer, x2 because
	 * 								 it's stereo
	 * framesPerBar .. n. of frames within a bar
	 * framesPerBeat . n. of frames within a beat */

	float seconds   = (60.0f / bpm) * beats;
	totalFrames     = G_Conf.samplerate * seconds * 2;
	framesPerBar    = totalFrames / bars;
	framesPerBeat   = totalFrames / beats;

	/* big troubles if frames are odd. */

	if (totalFrames % 2 != 0)
		totalFrames--;
	if (framesPerBar % 2 != 0)
		framesPerBar--;
	if (framesPerBeat % 2 != 0)
		framesPerBeat--;

	updateQuanto();

	/* realloc input virtual channel, if not NULL. TotalFrames is changed! */

	if (vChanInput != NULL)
		free(vChanInput);
	vChanInput = (float*) malloc(totalFrames * sizeof(float));
	if (!vChanInput)
		printf("[Mixer] vChanInput realloc error!");
}


/* ------------------------------------------------------------------ */


int Mixer::close() {
	running = false;
	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {
		if (chan[i] != NULL) {
			chanStatus[i] = STATUS_OFF;		  // for safety
			delete chan[i];
		}

		/* all vChans must be deallocated */

		free(vChan[i]);
	}

	/* free virtual input channel */

	free(vChanInput);
	free(vChanInToOut);

	return 1;
}


/* ------------------------------------------------------------------ */


bool Mixer::isSilent() {
	for (unsigned i=0; i<MAX_NUM_CHAN; i++)
		if (chanStatus[i] == STATUS_PLAY)
			return false;
	return true;
}


/* ------------------------------------------------------------------ */


void Mixer::rewind() {

	actualFrame = 0;
	G_beats     = 0;

	if (running)
		for (unsigned i=0; i<MAX_NUM_CHAN; i++)
			if (chan[i] != NULL) {

				/* don't rewind a SINGLE_ANY, unless it's in read-record-mode */

				if ((chanMode[i] & LOOP_ANY) || (chanRecStatus[i] == REC_READING && (chanMode[i] & SINGLE_ANY)))
					chanReset(i); //chanTracker[i] = chanStart[i];
			}
}


/* ------------------------------------------------------------------ */


void Mixer::updateQuanto() {

	/* big troubles if frames are odd. */

	if (quantize != 0)
		quanto = framesPerBeat / quantize;
	if (quanto % 2 != 0)
		quanto++;
}


/* ------------------------------------------------------------------ */


void Mixer::calcFadeoutStep(int ch) {

	/* how many frames are left before the end of the sample? Is there
	 * enough room for a complete fadeout? Should we shorten it? */

	unsigned ctracker = chanTracker[ch]*chanPitch[ch];

	if (chanEnd[ch] - ctracker < (1 / DEFAULT_FADEOUT_STEP) * 2)
		fadeoutStep[ch] = ceil((chanEnd[ch] - ctracker) / chanVolume[ch]) * 2;
	else
		fadeoutStep[ch] = DEFAULT_FADEOUT_STEP;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasLogicalSamples() {
	for (int i=0; i<MAX_NUM_CHAN; i++)
		if (chan[i] != NULL)
			if (chan[i]->isLogical)
				return true;
	return false;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasEditedSamples() {
	for (int i=0; i<MAX_NUM_CHAN; i++)
		if (chan[i] != NULL)
			if (chan[i]->isEdited)
				return true;
	return false;
}


/* ------------------------------------------------------------------ */


void Mixer::setPitch(int ch, float val) {

	/* if the pitch changes also chanStart/chanEnd must change accordingly
	 * and to do that we need the original (or previous) chanStart/chanEnd
	 * values (chanStartTrue and chanEndTrue). Formula:
	 *
	 * chanStart{pitched} = chanStart{Original} / newpitch */

	if (val == 1.0f) {
		chanStart[ch] = chanStartTrue[ch];
		chanEnd[ch]   = chanEndTrue[ch];
		chanPitch[ch] = 1.0f;
		return;
	}

	chanStart[ch] = (unsigned) floorf(chanStartTrue[ch] / val);
	chanEnd[ch]   = (unsigned) floorf(chanEndTrue[ch]   / val);

	chanPitch[ch] = val;

	/* even values please */

	if (chanStart[ch] % 2 != 0)	chanStart[ch]++;
	if (chanEnd[ch]   % 2 != 0)	chanEnd[ch]++;

	/* avoid overflow when changing pitch during play mode */

	if (chanTracker[ch] > chanEnd[ch])
		chanTracker[ch] = chanEnd[ch];
}


/* ------------------------------------------------------------------ */


void Mixer::setChanStart(int ch, unsigned val) {
	if (val % 2 != 0)
		val++;
	chanStartTrue[ch] = val;
	chanStart[ch]     = (unsigned) floorf(chanStartTrue[ch] / chanPitch[ch]);
	chanTracker[ch]   = chanStart[ch];
}


/* ------------------------------------------------------------------ */


void Mixer::setChanEnd(int ch, unsigned val) {
	if (val % 2 != 0)
		val++;
	chanEndTrue[ch] = val;
	chanEnd[ch]     = (unsigned) floorf(chanEndTrue[ch] / chanPitch[ch]);
}


/* ------------------------------------------------------------------ */


bool Mixer::mergeVirtualInput() {
	if (vChanInput == NULL) {
		puts("[Mixer] virtual input channel not alloc'd");
		return false;
	}
	else {
#ifdef WITH_VST
		G_PluginHost.processStackOffline(vChanInput, PluginHost::MASTER_IN, 0, totalFrames);
#endif
		int numFrames = totalFrames*sizeof(float);
		memcpy(chan[chanInput]->data, vChanInput, numFrames);
		memset(vChanInput, 0, numFrames); // clear vchan
		return true;
	}
}


/* ------------------------------------------------------------------ */


bool Mixer::isPlaying(int c) {
	return chanStatus[c] == STATUS_PLAY || chanStatus[c] == STATUS_ENDING;
}
