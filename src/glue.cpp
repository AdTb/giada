/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * How to know if you need another glue_ function? Ask yourself if the
 * new action will ever be called via MIDI or keyboard/mouse. If yes,
 * put it here.
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

#include "glue.h"

extern gdMainWindow *mainWin;
extern Mixer	   		 G_Mixer;
extern Patch     		 G_Patch;
extern Conf	 	   		 G_Conf;
extern bool 		 		 G_audio_status;
#ifdef WITH_VST
extern PluginHost		 G_PluginHost;
#endif


/* ------------------------------------------------------------------ */


int glue_loadChannel(int c, const char *fname, const char *fpath) {

	/* save the patch and take the last browser's dir in order to re-use it
	 * the next time */

	G_Conf.setPath(G_Conf.samplePath, fpath);

	int result = mh_loadChan(fname, c);

	if (result == SAMPLE_LOADED_OK)
		gu_trim_label(G_Mixer.chan[c]->name.c_str(), 28, mainWin->keyboard->sampleButton[c]);

	return result;
}


/* ------------------------------------------------------------------ */


int glue_loadPatch(const char *fname, const char *fpath, gProgress *status) {

	/* update browser's status bar with % 0.1 */

	status->show();
	status->value(0.1f);
	//Fl::check();
	Fl::wait(0);

	/* is it a valid patch? */

	int res = G_Patch.open(fname);
	if (res != PATCH_OPEN_OK)
		return res;

	/* reset the system. False = don't update the gui right now */

	glue_resetToInitState(false);

	status->value(0.2f);  // progress status: % 0.2
	//Fl::check();
	Fl::wait(0);

	/* mixerHandler will update the samples inside Mixer */

	mh_loadPatch();

	/* take the patch name and update the main window's title */

	G_Patch.getName();
	gu_update_win_label(G_Patch.name);

	status->value(0.4f);  // progress status: 0.4
	//Fl::check();
	Fl::wait(0);

	G_Patch.readRecs();
	status->value(0.6f);  // progress status: 0.6
	//Fl::check();
	Fl::wait(0);

#ifdef WITH_VST
	int resPlugins = G_Patch.readPlugins();
	status->value(0.8f);  // progress status: 0.8
	//Fl::check();
	Fl::wait(0);
#endif

	/* this one is vital: let recorder recompute the actions' positions if
	 * the current samplerate != patch samplerate */

	recorder::updateSamplerate(G_Conf.samplerate, G_Patch.samplerate);

	/* update gui */

	gu_update_controls();

	status->value(1.0f);  // progress status: 1.0 (done)
	//Fl::check();
	Fl::wait(0);

	/* save patchPath by taking the last dir of the broswer, in order to
	 * reuse it the next time */

	G_Conf.setPath(G_Conf.patchPath, fpath);

	printf("[glue] patch %s loaded\n", fname);

#ifdef WITH_VST
	if (resPlugins != 1)
		gdAlert("Some VST files were not loaded successfully.");
#endif

	/* lastly close all other windows */

	if (res)
		gu_closeAllSubwindows();

	return res;
}


/* ------------------------------------------------------------------ */


int glue_savePatch(const char *fullpath, const char *name) {

	if (G_Patch.write(fullpath, name) == 1) {
		strcpy(G_Patch.name, name);
		G_Patch.name[strlen(name)] = '\0';
		gu_update_win_label(name);
		printf("[glue] patch saved as %s\n", fullpath);
		return 1;
	}
	else
		return 0;
}


/* ------------------------------------------------------------------ */


int glue_saveSample(int ch, const char *fullpath) {
	if (G_Mixer.chan[ch]->writeData(fullpath)) {
		printf("[glue] sample %s saved\n", fullpath);
		return 1;
	}
	else {
		printf("[glue] error while saving sample %s!\n", fullpath);
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int glue_unloadChannel(int c) {
	mh_freeChan(c);
	recorder::clearChan(c);
	gu_resetChannel(c);
	return 1;
}


/* ------------------------------------------------------------------ */


void glue_setBpm(const char *v1, const char *v2) {
	char  buf[6];
	float value = atof(v1) + (atof(v2)/10);
	if (value < 20.0f)	{
		value = 20.0f;
		sprintf(buf, "20.0");
	}
	else
		sprintf(buf, "%s.%s", v1, !strcmp(v2, "") ? "0" : v2);

	/* a value such as atof("120.1") will never be 120.1 but 120.0999999,
	 * because of the rounding error. So we pass the real "wrong" value to
	 * G_Mixer and we show the nice looking (but fake) one to the GUI. */

	float old_bpm = G_Mixer.bpm;
	G_Mixer.bpm = value;
	G_Mixer.updateFrameBars();
	recorder::updateBpm(old_bpm, value, G_Mixer.quanto);
	mainWin->bpm->copy_label(buf);
	printf("[glue] Bpm changed to %s (real=%f)\n", buf, G_Mixer.bpm);
}


/* ------------------------------------------------------------------ */


void glue_setBeats(int beats, int bars, bool expand) {

	/* temp vars to store old data (they are necessary) */

	int      oldvalue = G_Mixer.beats;
	unsigned oldfpb		= G_Mixer.totalFrames;

	if (beats > MAX_BEATS)
		G_Mixer.beats = MAX_BEATS;
	else if (beats < 1)
		G_Mixer.beats = 1;
	else
		G_Mixer.beats = beats;

	/* update bars - bars cannot be greate than beats and must be a sub
	 * multiple of beats. If not, approximation to the nearest (and greater)
	 * value available. */

	if (bars > G_Mixer.beats)
		G_Mixer.bars = G_Mixer.beats;
	else if (bars <= 0)
		G_Mixer.bars = 1;
	else if (beats % bars != 0) {
		G_Mixer.bars = bars + (beats % bars);
		if (beats % G_Mixer.bars != 0) // it could be an odd value, let's check it (and avoid it)
			G_Mixer.bars = G_Mixer.bars - (beats % G_Mixer.bars);
	}
	else
		G_Mixer.bars = bars;

	G_Mixer.updateFrameBars();

	/* update recorded actions */

	if (expand) {
		if (G_Mixer.beats > oldvalue)
			recorder::expand(oldfpb, G_Mixer.totalFrames);
		//else if (G_Mixer.beats < oldvalue)
		//	recorder::shrink(G_Mixer.totalFrames);
	}

	char buf_batt[8];
	sprintf(buf_batt, "%d/%d", G_Mixer.beats, G_Mixer.bars);
	mainWin->beats->copy_label(buf_batt);

	/* in case the action editor is open */

	gu_refreshActionEditor();
}


/* ------------------------------------------------------------------ */


void glue_startSeq() {
	G_Mixer.running = true;
	mainWin->beat_stop->value(1);
	mainWin->beat_stop->redraw();
}


/* ------------------------------------------------------------------ */


void glue_stopSeq() {

	G_Mixer.running = false;

	/* kill loop channels and recs if "samplesStopOnSeqHalt" == true */

	if (G_Conf.chansStopOnSeqHalt) {
		for (int i=0; i<MAX_NUM_CHAN; i++) {
			if (G_Mixer.chanMode[i] & (LOOP_BASIC | LOOP_ONCE | LOOP_REPEAT))
				mh_killChan(i);

			/* when a channel has recs in play?
			 * Recorder has events for that channel
			 * G_Mixer has at least one sample in play
			 * Recorder's channel is active (altrimenti può capitare che
			 * si stoppino i sample suonati manualmente in un canale con rec
			 * disattivate) */

			if (recorder::chanEvents[i] && recorder::chanActive[i] && G_Mixer.chanStatus[i] == STATUS_PLAY)
				mh_killChan(i);
		}
	}

	/* what to do if we stop the sequencer and some action recs are active?
	 * Deactivate the button and delete any 'rec on' status */

	if (recorder::active) {
		recorder::active = false;
		mainWin->beat_rec->value(0);
		mainWin->beat_rec->redraw();
	}

	/* if input recs are active (who knows why) we must deactivate them.
	 * Someone might stop the sequencer while an input rec is running. */

	if (G_Mixer.chanInput != -1) {
		mh_stopInputRec();
		mainWin->input_rec->value(0);
		mainWin->input_rec->redraw();
	}

	mainWin->beat_stop->value(0);
	mainWin->beat_stop->redraw();
}


/* ------------------------------------------------------------------ */


void glue_rewindSeq() {
	mh_rewind();
}


/* ------------------------------------------------------------------ */


void glue_startRec() {
	if (G_audio_status == false)
		return;
	if (!G_Mixer.running)
		glue_startSeq();	        // start the sequencer for convenience
	recorder::active = true;
	mainWin->beat_rec->value(1);
	mainWin->beat_rec->redraw();
}


/* ------------------------------------------------------------------ */


void glue_stopRec() {

	/* stop the recorder and sort the new actions */

	recorder::active = false;
	recorder::sortActions();

	for (unsigned i=0; i<MAX_NUM_CHAN; i++)
		glue_setChannelWithActions(i);

	mainWin->beat_rec->value(0);
	mainWin->beat_rec->redraw();

	/* if acton editor is on */

	gu_refreshActionEditor();
}


/* ------------------------------------------------------------------ */


void glue_startReadingRecs(int c) {

	/* treatRecsAsLoops */

	if (G_Conf.treatRecsAsLoops)
		G_Mixer.chanRecStatus[c] = REC_WAITING;
	else
		recorder::enableRead(c);

	mainWin->keyboard->readActions[c]->value(1);
	mainWin->keyboard->readActions[c]->redraw();

}


/* ------------------------------------------------------------------ */


void glue_stopReadingRecs(int c) {

	/* if "treatRecsAsLoop" wait until the sequencer reaches beat 0, so put
	 * the channel in REC_ENDING status */

	if (G_Conf.treatRecsAsLoops)
		G_Mixer.chanRecStatus[c] = REC_ENDING;
	else
		recorder::disableRead(c);

	mainWin->keyboard->readActions[c]->value(0);
	mainWin->keyboard->readActions[c]->redraw();
}


/* ------------------------------------------------------------------ */


void glue_quantize(int val) {
	G_Mixer.quantize = val;
	G_Mixer.updateQuanto();
}


/* ------------------------------------------------------------------ */


void glue_setVol(int ch, float v) {
	G_Mixer.chanVolume[ch] = v;
	mainWin->keyboard->vol[ch]->value(v);
}


/* ------------------------------------------------------------------ */


void glue_setOutVol(float val) {
	G_Mixer.outVol = val;
}


/* ------------------------------------------------------------------ */


void glue_setInVol(float val) {
	G_Mixer.inVol = val;
}


/* ------------------------------------------------------------------ */


void glue_clearAllSamples() {
	G_Mixer.running = false;
	for (unsigned i=0; i<MAX_NUM_CHAN; i++)
		mh_freeChan(i);
	recorder::init();
	gu_update_controls();
	return;
}


/* ------------------------------------------------------------------ */


void glue_clearAllRecs() {
	recorder::init();
	gu_update_controls();
}


/* ------------------------------------------------------------------ */


void glue_resetToInitState(bool resetGui) {
	G_Mixer.running = false;
	for (unsigned i=0; i<MAX_NUM_CHAN; i++)
		mh_freeChan(i);
	recorder::init();
	G_Patch.setDefault();
	G_Mixer.init();
#ifdef WITH_VST
	G_PluginHost.freeAllStacks();
#endif
	if (resetGui)	gu_update_controls();
}


/* ------------------------------------------------------------------ */


void glue_startStopMetronome() {
	G_Mixer.metronome = !G_Mixer.metronome;
}


/* ------------------------------------------------------------------ */


void glue_setBeginEndChannel(gdEditor *win, int ch, int b, int e, bool recalc, bool check) {

	if (check) {
		if ((unsigned) e > G_Mixer.chan[ch]->size)
			e = G_Mixer.chan[ch]->size;
		if (b < 0)
			b = 0;
		if ((unsigned) b > G_Mixer.chan[ch]->size)
			b = (G_Mixer.chan[ch]->size)-2;
		if ((unsigned) b >= G_Mixer.chanEndTrue[ch])
			b = G_Mixer.chanStart[ch];
		if ((unsigned) e <= G_Mixer.chanStartTrue[ch])
			e = G_Mixer.chanEnd[ch];
	}

	/* print mono values */

	char tmp[16];
	sprintf(tmp, "%d", b/2);
	win->chanStart->value(tmp);

	tmp[0] = '\0';
	sprintf(tmp, "%d", e/2);
	win->chanEnd->value(tmp);

	G_Mixer.setChanStart(ch, b);
	G_Mixer.setChanEnd  (ch, e);

	/* recalc is not needed when the user drags the bars directly over the waveform */

	if (recalc) {
		win->wt->wave->recalcPoints();	// importante, altrimenti non si vedono
		win->wt->wave->redraw();
	}
}


/* ------------------------------------------------------------------ */


void glue_setBoost(gdEditor *win, int ch, float val, bool numeric) {
	if (numeric) {
		if (val > 20.0f)
			val = 20.0f;
		else if (val < 0.0f)
			val = 0.0f;

	  float linear = pow(10, (val / 20)); // linear = 10^(dB/20)

		G_Mixer.chanBoost[ch] = linear;

		char buf[10];
		sprintf(buf, "%.2f", val);
		win->boostNum->value(buf);
		win->boostNum->redraw();

		win->boost->value(linear);
		win->boost->redraw();       /// inutile
	}
	else {
		G_Mixer.chanBoost[ch] = val;
		char buf[10];
		sprintf(buf, "%.2f", 20*log10(val));
		win->boostNum->value(buf);
		win->boostNum->redraw();
	}
}


/* ------------------------------------------------------------------ */


void glue_setVolEditor(class gdEditor *win, int ch, float val, bool numeric) {
	if (numeric) {
		if (val > 0.0f)
			val = 0.0f;
		else if (val < -60.0f)
			val = -INFINITY;

	  float linear = pow(10, (val / 20)); // linear = 10^(dB/20)

		G_Mixer.chanVolume[ch] = linear;

		win->volume->value(linear);
		win->volume->redraw();

		char buf[10];
		if (val > -INFINITY)
			sprintf(buf, "%.2f", val);
		else
			sprintf(buf, "-inf");
		win->volumeNum->value(buf);
		win->volumeNum->redraw();

		mainWin->keyboard->vol[ch]->value(linear);
		mainWin->keyboard->vol[ch]->redraw();
	}
	else {
		G_Mixer.chanVolume[ch] = val;

		float dbVal = 20 * log10(val);
		char buf[10];
		if (dbVal > -INFINITY)
			sprintf(buf, "%.2f", dbVal);
		else
			sprintf(buf, "-inf");

		win->volumeNum->value(buf);
		win->volumeNum->redraw();
		mainWin->keyboard->vol[ch]->value(val);
		mainWin->keyboard->vol[ch]->redraw();
	}
}


/* ------------------------------------------------------------------ */


void glue_writeMute(int ch, bool gui) {

	int action;
	int button;

	if (!G_Mixer.chanMute[ch]) {
		mh_muteChan(ch);
		action = ACTION_MUTEON;
		button = 1;
	}
	else {
		mh_unmuteChan(ch);
		action = ACTION_MUTEOFF;
		button = 0;
	}

	/* recording mute actions, only if Recorder can record. */

	if (recorder::canRec(ch)) {

		/* if recording an ACTION_MUTEOFF we want to avoid ring loops. Let's
		 * look for the ACTION_MUTEON. The process for an ACTION_MUTEON
		 * recording is actually straightforward. */

		if (action == ACTION_MUTEOFF) {

			int frame_a = recorder::getStartActionFrame(ch, ACTION_MUTEON, G_Mixer.actualFrame);
			int frame_b = G_Mixer.actualFrame;

			if (frame_a == -1)  // ring loop!
				recorder::rec(ch, action, G_Mixer.totalFrames);
			else
			if (frame_a == frame_b)
				recorder::deleteAction(ch, frame_b, ACTION_MUTEON);
			else
			recorder::rec(ch, action, G_Mixer.actualFrame);
		}
		else  // if action == mute_on
			recorder::rec(ch, action, G_Mixer.actualFrame);
	}

	/* if the caller is mixer/keyboard/MIDI (not the GUI) we turn on or
	 * off the button. */

	if (!gui)
		mainWin->keyboard->mute[ch]->value(button);
}


/* ------------------------------------------------------------------ */


void glue_readMute(int ch, int type) {
	if (type == ACTION_MUTEON) {
		mh_muteChan(ch);
		mainWin->keyboard->mute[ch]->value(1);
	}
	else {
		mh_unmuteChan(ch);
		mainWin->keyboard->mute[ch]->value(0);
	}
}


/* ------------------------------------------------------------------ */


void glue_setPanning(class gdEditor *win, int ch, float val) {

	if (val < 1.0f) {
		G_Mixer.chanPanLeft[ch]  = 1.0f;
		G_Mixer.chanPanRight[ch] = 0.0f + val;

		char buf[8];
		sprintf(buf, "%d L", abs((G_Mixer.chanPanRight[ch] * 100.0f) - 100));
		win->panNum->value(buf);
	}
	else if (val == 1.0f) {
		G_Mixer.chanPanLeft[ch]  = 1.0f;
		G_Mixer.chanPanRight[ch] = 1.0f;
	  win->panNum->value("C");
	}
	else {
		G_Mixer.chanPanLeft[ch]  = 2.0f - val;
		G_Mixer.chanPanRight[ch] = 1.0f;

		char buf[8];
		sprintf(buf, "%d R", abs((G_Mixer.chanPanLeft[ch] * 100.0f) - 100));
		win->panNum->value(buf);
	}
	win->panNum->redraw();
}


/* ------------------------------------------------------------------ */


int glue_startInputRec() {

	if (G_audio_status == false)
		return -1;

	if (G_Mixer.chanInput != -1)			// if there's another recording active
		return 1;

	if (!G_Mixer.running) {
		glue_startSeq();
		mainWin->beat_stop->value(1);
	}

	int ch = mh_startInputRec();
	if (ch == -1)	                    // no chans available
		return 0;

	glue_setVol(ch, 1.0f);
	gu_trim_label(G_Mixer.chan[ch]->name.c_str(), 28, mainWin->keyboard->sampleButton[ch]);

	mainWin->input_rec->value(1);
	mainWin->input_rec->redraw();
	mainWin->beat_stop->redraw();
	return 1;

}


/* ------------------------------------------------------------------ */


int glue_stopInputRec() {

	int ch = mh_stopInputRec();

	if (G_Mixer.chanMode[ch] & (LOOP_BASIC | LOOP_ONCE | LOOP_REPEAT))
		mh_startChan(ch);

	mainWin->input_rec->value(0);
	mainWin->input_rec->redraw();
	return 1;
}


/* ------------------------------------------------------------------ */


int glue_saveProject(const char *folderPath, const char *projName) {

	if (gIsProject(folderPath)) {
		puts("[glue] the project folder already exists");
		// don't exit
	}
	else if (!gMkdir(folderPath)) {
		puts("[glue] unable to make project directory!");
		return 0;
	}

	/* copy all samples inside the folder. Takes and logical ones are saved
	 * with glue_saveSample() */

	for (int i=0; i<MAX_NUM_CHAN; i++) {

		if (G_Mixer.chan[i] == NULL)
			continue;

		/* update the new samplePath: everything now comes from the project folder */

		char samplePath[PATH_MAX];
		sprintf(samplePath, "%s/%s.wav", folderPath, G_Mixer.chan[i]->name.c_str());

		/* remove any existing file */

		if (gFileExists(samplePath))
			remove(samplePath);
		if (glue_saveSample(i, samplePath))
			G_Mixer.chan[i]->pathfile = samplePath;
	}

	std::string projNameClean = stripExt(projName);
	char gptcPath[PATH_MAX];
	sprintf(gptcPath, "%s/%s.gptc", folderPath, projNameClean.c_str());
	glue_savePatch(gptcPath, projName);

	return 1;
}


/* ------------------------------------------------------------------ */


void glue_keyPress(int c, bool ctrl, bool shift) {

	/* -- case CTRL --------------------------------------------------- */
	if (ctrl)
		glue_writeMute(c, false);

	/* -- case SHIFT------------------------------------------------------
	 * action recording on:
	 * 		if seq is playing, rec a killchan
	 * action recording off:
	 * 		if chan has recorded events:
	 * 		|	 if seq is playing OR channel 'c' is stopped, de/activate recs
	 * 		|	 else kill chan
	 *		else kill chan */
	else
	if (shift)	{
		if (recorder::active) {
			if (G_Mixer.running) {
				mh_killChan(c);
				if (recorder::canRec(c) && !(G_Mixer.chanMode[c] & LOOP_ANY))   // don't record killChan actions for LOOP channels
					recorder::rec(c, ACTION_KILLCHAN, G_Mixer.actualFrame);
			}
		}
		else {
			if (recorder::chanEvents[c]) {
				if (G_Mixer.running || G_Mixer.chanStatus[c] == STATUS_OFF)
					recorder::chanActive[c] ? glue_stopReadingRecs(c) : glue_startReadingRecs(c);
				else
					mh_killChan(c);
			}
			else
				mh_killChan(c);
		}
	}

	/* -- case no modifier -------------------------------------------- */
	else {
		mh_startChan(c);

		/* record now if the quantizer is off, otherwise let mixer to handle it
		 * when a quantoWait has passed. Moreover, KEYPRESS and KEYREL are
		 * meaningless for loop modes */

		if (G_Mixer.quantize == 0 && recorder::canRec(c) && !(G_Mixer.chanMode[c] & LOOP_ANY))
			recorder::rec(c, ACTION_KEYPRESS, G_Mixer.actualFrame);
	}

	/* the GUI update is done by gui_refresh() */
}


/* ------------------------------------------------------------------ */


void glue_keyRelease(int c, bool ctrl, bool shift) {

	if (!ctrl && !shift) {
		mh_stopChan(c);

		/* we record a key release only if channel is single_press. For any
		 * other mode the key release is meaningless. */

		if (G_Mixer.chanMode[c] == SINGLE_PRESS && recorder::canRec(c)) {

			int frame_a = recorder::getStartActionFrame(c, ACTION_KEYPRESS, G_Mixer.actualFrame);
			int frame_b = G_Mixer.actualFrame;

			/* we must avoid any ring loop effect, i.e. an action key_press
			 * that starts at frame N and the corresponding key_release at
			 * frame M, with M <= N. In that case the key_release is recorded
			 * right at the end of the sequencer. Recorder::getStartActionFrame
			 * returns -1 in case of a suspected ring loop. */

			if (frame_a == -1)
				recorder::rec(c, ACTION_KEYREL, G_Mixer.totalFrames);

			/* now we avoid frame_a == frame_b: it happens with a quick press
			 * and release of the button. In that case we delete frame_a. */

			else
			if (frame_a == -2)
				recorder::deleteAction(c, frame_b, ACTION_KEYPRESS);

			else {
				recorder::rec(c, ACTION_KEYREL, frame_b);
			}
		}
	}

	/* the GUI update is done by gui_refresh() */

}


/* ------------------------------------------------------------------ */


void glue_setPitch(class gdEditor *win, int ch, float val, bool numeric) {
	if (numeric) {
		if (val <= 0.0f)
			val = 0.1000f;
		if (val > 2.0f)
			val = 2.0000f;
		win->pitch->value(val);
	}

	G_Mixer.setPitch(ch, val);

	char buf[16];
	sprintf(buf, "%.4f", val);
	win->pitchNum->value(buf);
	win->pitchNum->redraw();
}


/* ------------------------------------------------------------------ */


void glue_setChannelWithActions(int ch) {
	if (recorder::chanEvents[ch]) {
		recorder::chanActive[ch] = true;
		mainWin->keyboard->addActionButton(ch, true); // true = button on
	}
	else {
		recorder::chanActive[ch] = false;
		mainWin->keyboard->remActionButton(ch);
	}
}


/* ------------------------------------------------------------------ */


/* never expand or shrink recordings (last param of setBeats = false):
 * this is live manipulation */

void glue_beatsMultiply() {
	glue_setBeats(G_Mixer.beats*2, G_Mixer.bars, false);
}

void glue_beatsDivide() {
	glue_setBeats(G_Mixer.beats/2, G_Mixer.bars, false);
}
