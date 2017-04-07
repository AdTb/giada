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


#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/channel.h"
#include "../core/pluginHost.h"
#include "../core/plugin.h"
#include "../core/conf.h"
#include "../core/patch.h"
#include "../core/patch_DEPR_.h" // TODO - remove, used only for DEPR calls
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/clock.h"
#include "../core/wave.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../utils/fs.h"
#include "../gui/elems/mainWindow/keyboard/column.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/dialogs/gd_browser.h"
#include "main.h" // TODO - remove, used only for DEPR calls
#include "channel.h"
#include "storage.h"


extern gdMainWindow *G_MainWin;
extern Patch_DEPR_   G_Patch_DEPR_; // TODO - remove, used only for DEPR calls


using std::string;
using std::vector;
using namespace giada;


#ifdef WITH_VST

static void __glue_fillPatchGlobalsPlugins__(vector <Plugin *> *host, vector<patch::plugin_t> *patch)
{
	for (unsigned i=0; i<host->size(); i++) {
		Plugin *pl = host->at(i);
		patch::plugin_t ppl;
		ppl.path = pl->getUniqueId();
		ppl.bypass = pl->isBypassed();
		int numParams = pl->getNumParameters();
		for (int k=0; k<numParams; k++)
			ppl.params.push_back(pl->getParameter(k));
		patch->push_back(ppl);
	}
}

#endif


/* -------------------------------------------------------------------------- */


static void __glue_fillPatchColumns__()
{
	for (unsigned i=0; i<G_MainWin->keyboard->getTotalColumns(); i++) {
		geColumn *gCol = G_MainWin->keyboard->getColumn(i);
		patch::column_t pCol;
		pCol.index = gCol->getIndex();
		pCol.width = gCol->w();
		for (int k=0; k<gCol->countChannels(); k++) {
			Channel *colChannel = gCol->getChannel(k);
			for (unsigned j=0; j<mixer::channels.size(); j++) {
				Channel *mixerChannel = mixer::channels.at(j);
				if (colChannel == mixerChannel) {
					pCol.channels.push_back(mixerChannel->index);
					break;
				}
			}
		}
		patch::columns.push_back(pCol);
	}
}


/* -------------------------------------------------------------------------- */


static void __glue_fillPatchChannels__(bool isProject)
{
	for (unsigned i=0; i<mixer::channels.size(); i++) {
		mixer::channels.at(i)->writePatch(i, isProject);
	}
}


/* -------------------------------------------------------------------------- */


static void __glue_fillPatchGlobals__(const string &name)
{
	patch::version      = G_VERSION_STR;
	patch::versionMajor = G_VERSION_MAJOR;
	patch::versionMinor = G_VERSION_MINOR;
	patch::versionPatch = G_VERSION_PATCH;
	patch::name         = name;
	patch::bpm          = clock::getBpm();
	patch::bars         = clock::getBars();
	patch::beats        = clock::getBeats();
	patch::quantize     = clock::getQuantize();
	patch::masterVolIn  = mixer::inVol;
  patch::masterVolOut = mixer::outVol;
  patch::metronome    = mixer::metronome;

#ifdef WITH_VST

	__glue_fillPatchGlobalsPlugins__(pluginHost::getStack(pluginHost::MASTER_IN),
			&patch::masterInPlugins);
	__glue_fillPatchGlobalsPlugins__(pluginHost::getStack(pluginHost::MASTER_OUT),
			&patch::masterOutPlugins);

#endif
}


/* -------------------------------------------------------------------------- */


static bool __glue_savePatch__(const string &fullPath, const string &name,
		bool isProject)
{
	patch::init();

	__glue_fillPatchGlobals__(name);
	__glue_fillPatchChannels__(isProject);
	__glue_fillPatchColumns__();

	if (patch::write(fullPath)) {
		gu_updateMainWinLabel(name);
		gu_log("[glue_savePatch] patch saved as %s\n", fullPath.c_str());
		return true;
	}
	return false;
}


/* -------------------------------------------------------------------------- */


void glue_savePatch(void *data)
{
	gdSaveBrowser *browser = (gdSaveBrowser*) data;
	string name            = browser->getName();
	string fullPath        = browser->getCurrentPath() + G_SLASH + gu_stripExt(name) + ".gptc";

	if (name == "") {
		gdAlert("Please choose a file name.");
		return;
	}

	if (gu_fileExists(fullPath))
		if (!gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	if (__glue_savePatch__(fullPath, name, false)) {  // false == not a project
		conf::patchPath = gu_dirname(fullPath);
		browser->do_callback();
	}
	else
		gdAlert("Unable to save the patch!");
}


/* -------------------------------------------------------------------------- */


void glue_loadPatch(void *data)
{
	gdLoadBrowser *browser = (gdLoadBrowser*) data;
	string fullPath        = browser->getSelectedItem();
	bool isProject         = gu_isProject(browser->getSelectedItem());

	browser->showStatusBar();

	gu_log("[glue] loading %s...\n", fullPath.c_str());

	string fileToLoad = fullPath;  // patch file to read from
	string basePath   = "";        // base path, in case of reading from a project
	if (isProject) {
		fileToLoad = fullPath + G_SLASH + gu_stripExt(gu_basename(fullPath)) + ".gptc";
		basePath   = fullPath + G_SLASH;
	}

	/* try to load the new JSON-based patch. If it fails, fall back to deprecated
	* one. */

	int  res = patch::read(fileToLoad);
	bool deprecated = false;

	if (res == PATCH_UNREADABLE) {
		gu_log("[glue] failed reading JSON-based patch. Trying with the deprecated method\n");
		deprecated = true;
		res = glue_loadPatch__DEPR__(gu_basename(fileToLoad).c_str(), fileToLoad.c_str(),
				browser->getStatusBar(), isProject);
	}

	if (res != PATCH_READ_OK) {
		if (res == PATCH_UNREADABLE)
			isProject ? gdAlert("This project is unreadable.") : gdAlert("This patch is unreadable.");
		else
		if (res == PATCH_INVALID)
			isProject ? gdAlert("This project is not valid.") : gdAlert("This patch is not valid.");

		browser->hideStatusBar();
		return;
	}
	else
	if (deprecated) {
		browser->do_callback();
		return;
	}

	/* close all other windows. This prevents segfault if plugin
	 * windows GUIs are on. */

	gu_closeAllSubwindows();

	/* reset the system. False(1): don't update the gui right now. False(2): do
	 * not create empty columns. */

	glue_resetToInitState(false, false);

	browser->setStatusBar(0.1f);
	//__glue_setProgressBar__(status, 0.1f);

	/* Add common stuff, columns and channels. Also increment the progress bar
	 * by 0.8 / total_channels steps.  */

	float steps = 0.8 / patch::channels.size();
	for (unsigned i=0; i<patch::columns.size(); i++) {
		patch::column_t *col = &patch::columns.at(i);
		G_MainWin->keyboard->addColumn(col->width);
		for (unsigned k=0; k<patch::channels.size(); k++) {
			if (patch::channels.at(k).column == col->index) {
				Channel *ch = glue_addChannel(patch::channels.at(k).column,
					patch::channels.at(k).type);
				ch->readPatch(basePath, k, &mixer::mutex_plugins, conf::samplerate,
          conf::rsmpQuality);
			}
			browser->setStatusBar(steps);
		}
	}

	/* fill Mixer */

	mh::readPatch();

	/* let recorder recompute the actions' positions if the current
	 * samplerate != patch samplerate */

	recorder::updateSamplerate(conf::samplerate, patch::samplerate);

	/* save patchPath by taking the last dir of the broswer, in order to
	 * reuse it the next time */

	conf::patchPath = gu_dirname(fullPath);

	/* refresh GUI */

	gu_updateControls();
	gu_updateMainWinLabel(patch::name);

	browser->setStatusBar(0.1f);
	//__glue_setProgressBar__(status, 1.0f);

	gu_log("[glue] patch loaded successfully\n");

#ifdef WITH_VST

	if (pluginHost::hasMissingPlugins())
		gdAlert("Some plugins were not loaded successfully.\nCheck the plugin browser to know more.");

#endif

	browser->do_callback();
}


/* -------------------------------------------------------------------------- */


int glue_loadPatch__DEPR__(const char *fname, const char *fpath, gProgress *status, bool isProject)
{
	/* update browser's status bar with % 0.1 */

	status->show();
	status->value(0.1f);
	//Fl::check();
	Fl::wait(0);

	/* is it a valid patch? */

	int res = G_Patch_DEPR_.open(fpath);
	if (res != PATCH_READ_OK)
		return res;

	/* close all other windows. This prevents segfault if plugin windows
	 * GUI are on. */

	if (res)
		gu_closeAllSubwindows();

	/* reset the system. False(1): don't update the gui right now. False(2): do
	 * not create empty columns. */

	glue_resetToInitState(false, false);

	status->value(0.2f);  // progress status: % 0.2
	//Fl::check();
	Fl::wait(0);

	/* mixerHandler will update the samples inside Mixer */

	mh::loadPatch_DEPR_(isProject, gu_dirname(fpath).c_str());

	/* take the patch name and update the main window's title */

	G_Patch_DEPR_.getName();
	gu_updateMainWinLabel(G_Patch_DEPR_.name);

	status->value(0.4f);  // progress status: 0.4
	//Fl::check();
	Fl::wait(0);

	G_Patch_DEPR_.readRecs();
	status->value(0.6f);  // progress status: 0.6
	//Fl::check();
	Fl::wait(0);

#ifdef WITH_VST
	int resPlugins = G_Patch_DEPR_.readPlugins();
	status->value(0.8f);  // progress status: 0.8
	//Fl::check();
	Fl::wait(0);
#endif

	/* this one is vital: let recorder recompute the actions' positions if
	 * the current samplerate != patch samplerate */

	recorder::updateSamplerate(conf::samplerate, G_Patch_DEPR_.samplerate);

	/* update gui */

	gu_updateControls();

	status->value(1.0f);  // progress status: 1.0 (done)
	//Fl::check();
	Fl::wait(0);

	/* save patchPath by taking the last dir of the broswer, in order to
	 * reuse it the next time */

	conf::patchPath = gu_dirname(fpath).c_str();

	gu_log("[glue] patch %s loaded\n", fname);

#ifdef WITH_VST
	if (resPlugins != 1)
		gdAlert("Some VST plugins were not loaded successfully.");
#endif

	gdAlert("This patch is using a deprecated format.\nPlease save it again to store it properly.");

	return res;
}


/* -------------------------------------------------------------------------- */


void glue_saveProject(void *data)
{
	gdSaveBrowser *browser = (gdSaveBrowser*) data;
	string name            = browser->getName();
	string folderPath      = browser->getCurrentPath(); //browser->getSelectedItem();
	string fullPath        = folderPath + G_SLASH + gu_stripExt(name) + ".gprj";

	if (name == "") {
		gdAlert("Please choose a project name.");
		return;
	}

	if (gu_isProject(fullPath) && !gdConfirmWin("Warning", "Project exists: overwrite?"))
		return;

	if (!gu_dirExists(fullPath) && !gu_mkdir(fullPath)) {
		gu_log("[glue_saveProject] unable to make project directory!\n");
		return;
	}

	gu_log("[glue_saveProject] project dir created: %s\n", fullPath.c_str());

	/* copy all samples inside the folder. Takes and logical ones are saved
	 * via glue_saveSample() */

	for (unsigned i=0; i<mixer::channels.size(); i++) {

		if (mixer::channels.at(i)->type == CHANNEL_MIDI)
			continue;

		SampleChannel *ch = (SampleChannel*) mixer::channels.at(i);

		if (ch->wave == nullptr)
			continue;

		/* update the new samplePath: everything now comes from the project
		 * folder (folderPath). Also remove any existing file. */

		string samplePath = fullPath + G_SLASH + ch->wave->basename(true);

		if (gu_fileExists(samplePath))
			remove(samplePath.c_str());
		if (ch->save(samplePath.c_str()))
			ch->wave->pathfile = samplePath;
	}

	string gptcPath = fullPath + G_SLASH + gu_stripExt(name) + ".gptc";
	if (__glue_savePatch__(gptcPath, name, true)) // true == it's a project
		browser->do_callback();
	else
		gdAlert("Unable to save the project!");
}


/* -------------------------------------------------------------------------- */


void glue_loadSample(void *data)
{
	gdLoadBrowser *browser = (gdLoadBrowser*) data;
	string fullPath        = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	int res = glue_loadChannel((SampleChannel*) browser->getChannel(), fullPath.c_str());

	if (res == SAMPLE_LOADED_OK) {
		conf::samplePath = gu_dirname(fullPath);
		browser->do_callback();
		G_MainWin->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
	}
	else
		G_MainWin->keyboard->printChannelMessage(res);
}


/* -------------------------------------------------------------------------- */


void glue_saveSample(void *data)
{
	gdSaveBrowser *browser = (gdSaveBrowser*) data;
	string name            = browser->getName();
	string folderPath      = browser->getSelectedItem();

	if (name == "") {
		gdAlert("Please choose a file name.");
		return;
	}

	/* bruteforce check extension. */

	string filePath = folderPath + G_SLASH + gu_stripExt(name) + ".wav";

	if (gu_fileExists(filePath))
		if (!gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	if (((SampleChannel*)browser->getChannel())->save(filePath.c_str())) {
		conf::samplePath = gu_dirname(folderPath);
		browser->do_callback();
	}
	else
		gdAlert("Unable to save this sample!");
}
