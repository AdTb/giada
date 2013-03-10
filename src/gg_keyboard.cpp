/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
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


#include "gg_keyboard.h"
#include "gd_browser.h"
#include "const.h"
#include "mixer.h"
#include "wave.h"
#include "gd_editor.h"
#include "conf.h"
#include "patch.h"
#include "gd_mainWindow.h"
#include "graphics.h"
#include "glue.h"
#include "recorder.h"
#include "gd_warnings.h"
#include "pluginHost.h"
#include "channel.h"


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H, const char* L, channel *ch)
: Fl_Group(X, Y, W, H, L), ch(ch)
{
	begin();
	button = new gButton (x(), y(), 20, 20);
	status = new gStatus (button->x()+button->w()+4, y(), 20, 20, ch);
#if defined(WITH_VST)
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 237, 20, "-- no sample --");
	mute         = new gClick  (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	fx           = new gButton (mute->x()+mute->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial   (fx->x()+fx->w()+4, y(), 20, 20);
#else
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 261, 20, "-- no sample --");
	mute         = new gClick  (sampleButton->x()+sampleButton->w()+4, y(), 20,  20, "", muteOff_xpm, muteOn_xpm);
	vol          = new gDial   (mute->x()+mute->w()+4, y(), 20, 20);
#endif
	modeBox      = new gModeBox(vol->x()+vol->w()+4, y(), 20, 20, ch);
	readActions  = NULL; // no rec button at start
	end();

	if (ch->wave)
		gu_trim_label(ch->wave->name.c_str(), 28, sampleButton);

	button->callback(cb_button, (void*)this);
	/*
	char buf[2]; sprintf(buf, "%c", arkeys[i]);
	button->copy_label(buf);
	button->key = arkeys[i];
	button->id  = i;
	*/

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);
	sampleButton->callback(cb_openChanMenu, (void*)this);
	vol->callback(cb_change_vol, (void*)this);

	ch->guiChannel = this;
}


/* ------------------------------------------------------------------ */


void gChannel::cb_button      (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_button(); }
void gChannel::cb_mute        (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_mute(); }
void gChannel::cb_openChanMenu(Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_openChanMenu(); }
void gChannel::cb_change_vol  (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_change_vol(); }
void gChannel::cb_readActions (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_readActions(); }
#ifdef WITH_VST
void gChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */


void gChannel::__cb_button() {

	/*
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
	*/
	glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gChannel::__cb_openFxWindow() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif


/* ------------------------------------------------------------------ */


void gChannel::reset() {
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- no sample --");
	remActionButton();
	sampleButton->redraw();
	status->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_mute() {
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_openChanMenu() {

	if (ch == NULL) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	/* if you're recording (actions or input) no menu is allowed; you can't
	 * do anything, especially deallocate the channel */

	if (G_Mixer.chanInput == ch || recorder::active)
		return;

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	/* the following is a trash workaround for a FLTK menu. We need a gMenu
	 * widget asap */

	Fl_Menu_Item rclick_menu[] = {
		{"Load new sample..."},
		{"Export sample to file..."},
		{"Edit sample..."},
		{"Edit actions..."},
		{"Clear actions", 0, 0, 0, FL_SUBMENU},
			{"All"},
			{"Mute"},
			{"Start/Stop"},
			{0},
		{"Free channel"},
		{"Delete channel"},
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[4].deactivate();

	/* no 'clear all actions' or 'clear start/stop actions' for those channels
	 * in loop mode: they can only have mute actions. */

	if (ch->mode & LOOP_ANY) {
		rclick_menu[5].deactivate();
		rclick_menu[7].deactivate();
	}

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Load new sample...") == 0) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Edit sample...") == 0) {
		gu_openSubWindow(mainWin, new gdEditor("title", ch), WID_SAMPLE_EDITOR); /// FIXME title it's up to gdEditor
		return;
	}

	if (strcmp(m->label(), "Export sample to file...") == 0) {
		openBrowser(BROWSER_SAVE_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Delete channel") == 0) {
		glue_deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Free channel") == 0) {
		if (ch->status == STATUS_PLAY) {
			if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
				return;
		}
		else if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
			return;

		glue_freeChannel(ch);

		/* delete any related subwindow */

		mainWin->delSubWindow(WID_FILE_BROWSER);
		mainWin->delSubWindow(WID_ACTION_EDITOR);
		mainWin->delSubWindow(WID_SAMPLE_EDITOR);
		mainWin->delSubWindow(WID_FX_LIST);

		return;
	}

	if (strcmp(m->label(), "Mute") == 0) {
		if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_MUTEON | ACTION_MUTEOFF);
		if (!ch->hasActions)
			remActionButton();

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
		if (!ch->hasActions)
			remActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		remActionButton();
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}
}


/* ------------------------------------------------------------------ */


void gChannel::openBrowser(int type) {
	const char *title = "";
	switch (type) {
		case BROWSER_LOAD_SAMPLE:
			title = "Browse Sample";
			break;
		case BROWSER_SAVE_SAMPLE:
			title = "Save Sample";
			break;
		case -1:
			title = "Edit Sample";
			break;
	}
	gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, ch, type);
	gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_change_vol() {
	glue_setVolMainWin(ch, vol->value());
}


/* ------------------------------------------------------------------ */


void gChannel::addActionButton(bool status) {

	/* quit if 'R' exists yet. */

	if (readActions != NULL)
		return;

	sampleButton->size(sampleButton->w()-24, sampleButton->h());

	redraw();

	readActions = new gClick(sampleButton->x() + sampleButton->w() + 4, sampleButton->y(), 20, 20, "", readActionOff_xpm, readActionOn_xpm);
	readActions->type(FL_TOGGLE_BUTTON);
	readActions->value(status);
	readActions->callback(cb_readActions, (void*)this);
	add(readActions);

	/* hard redraw: there's no other way to avoid glitches when moving
	 * the 'R' button */

	mainWin->keyboard->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::remActionButton() {
	if (readActions == NULL)
		return;

	remove(readActions);		// delete from Keyboard group (FLTK)
	//delete readActions[c];  // delete (C++)
	readActions = NULL;

	sampleButton->size(sampleButton->w()+24, sampleButton->h());
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_readActions() {
	ch->readActions ? glue_stopReadingRecs(ch) : glue_startReadingRecs(ch);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


Keyboard::Keyboard(int X, int Y, int W, int H, const char *L)
: Fl_Scroll(X, Y, W, H, L),
	bckspcPressed(false),
	endPressed(false),
	spacePressed(false)
{
	color(COLOR_BG_MAIN);
	type(Fl_Scroll::VERTICAL);
	scrollbar.color(COLOR_BG_0);
	scrollbar.selection_color(COLOR_BG_1);
	scrollbar.labelcolor(COLOR_BD_1);
	scrollbar.slider(G_BOX);

	gChannelsL  = new Fl_Group(x(), y(), (w()/2)-16, 0);
	gChannelsR  = new Fl_Group(gChannelsL->x()+gChannelsL->w()+8, y(), (w()/2)-16, 0);
	addChannelL = new gClick(gChannelsL->x(), gChannelsL->y()+gChannelsL->h(), gChannelsL->w(), 20, "Add new Left Channel");
	addChannelR = new gClick(gChannelsR->x(), gChannelsR->y()+gChannelsR->h(), gChannelsR->w(), 20, "Add new Right Channel");

	//updateChannels();

	/* begin() - end() don't work well here, with sub-Fl_Group */

	add(addChannelL);
	add(addChannelR);
	add(gChannelsL);
	add(gChannelsR);

	//gChannelsL->box(FL_BORDER_BOX);
	//gChannelsR->box(FL_BORDER_BOX);

	gChannelsL->resizable(NULL);
	gChannelsR->resizable(NULL);


	addChannelL->callback(cb_addChannelL, (void*) this);
	addChannelR->callback(cb_addChannelR, (void*) this);
}


/* ------------------------------------------------------------------ */


void Keyboard::freeChannel(struct channel *ch) {
	ch->guiChannel->reset();
}


/* ------------------------------------------------------------------ */


void Keyboard::deleteChannel(struct channel *ch) {
	Fl::lock();
	ch->guiChannel->hide();
	gChannelsR->remove(ch->guiChannel);
	gChannelsL->remove(ch->guiChannel);
	delete ch->guiChannel;
	ch->guiChannel = NULL;
	Fl::unlock();
}


/* ------------------------------------------------------------------ */


void Keyboard::updateChannel(struct channel *ch) {
	gu_trim_label(ch->wave->name.c_str(), 28, ch->guiChannel->sampleButton);
}


/* ------------------------------------------------------------------ */


gChannel *Keyboard::getChannel(struct channel *ch) {
	for (int i=0; i<gChannelsL->children(); i++) {
		gChannel *gch = (gChannel*) gChannelsL->child(i);
		if (gch->ch == ch)
			return gch;
	}
	for (int i=0; i<gChannelsR->children(); i++) {
		gChannel *gch = (gChannel*) gChannelsR->child(i);
		if (gch->ch == ch)
			return gch;
	}
	return NULL;
}


/* ------------------------------------------------------------------ */


void Keyboard::updateChannels(char side) {

	Fl_Group *group;
	gClick   *add;

	if (side == 0)	{
		group = gChannelsL;
		add   = addChannelL;
	}
	else {
		group = gChannelsR;
		add   = addChannelR;
	}

	printf("[keyboard::updateChannels] side %d has %d widgets\n", side, group->children());

	for (int i=0; i<group->children(); i++) {
		gChannel *gch = (gChannel*) group->child(i);
		gch->position(gch->x(), y()+(i*24));
	}
	group->size(group->w(), group->children()*24);
	add->position(add->x(), group->y()+group->h());

	redraw();
}


/* ------------------------------------------------------------------ */


void Keyboard::cb_addChannelL(Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_addChannelL(); }
void Keyboard::cb_addChannelR(Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_addChannelR(); }


/* ------------------------------------------------------------------ */


void Keyboard::__cb_addChannelL() {
	channel  *ch  = G_Mixer.loadChannel(NULL, 0);
	gChannel *gch = new gChannel(gChannelsL->x(), gChannelsL->y() + gChannelsL->children() * 24, gChannelsL->w(), 20, NULL, ch);

	gChannelsL->add(gch);
	gChannelsL->size(gChannelsL->w(), gChannelsL->children() * 24);
	addChannelL->position(gChannelsL->x(), gChannelsL->y()+gChannelsL->h());
	redraw();
}

void Keyboard::__cb_addChannelR() {
	channel  *ch  = G_Mixer.loadChannel(NULL, 1);
	gChannel *gch = new gChannel(gChannelsR->x(), gChannelsR->y() + gChannelsR->children() * 24, gChannelsR->w(), 20, NULL, ch);

	gChannelsR->add(gch);
	gChannelsR->size(gChannelsR->w(), gChannelsR->children() * 24);
	addChannelR->position(gChannelsR->x(), gChannelsR->y()+gChannelsR->h());
	redraw();
}


/* ------------------------------------------------------------------ */


int Keyboard::handle(int e) {
	int ret = Fl_Group::handle(e);  // assume the buttons won't handle the Keyboard events
	switch (e) {
		case FL_FOCUS:
		case FL_UNFOCUS: {
			ret = 1;                	// enables receiving Keyboard events
			break;
		}
		case FL_SHORTCUT:           // in case widget that isn't ours has focus
		case FL_KEYDOWN:            // Keyboard key pushed
		case FL_KEYUP: {            // Keyboard key released

			/* rewind session. Avoid retrigs */

			if (e == FL_KEYDOWN) {
				if (Fl::event_key() == FL_BackSpace && !bckspcPressed) {
					bckspcPressed = true;
					glue_rewindSeq();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_End && !endPressed) {
					endPressed = true;
					G_Mixer.chanInput == NULL ? glue_startInputRec() : glue_stopInputRec();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_Enter && !enterPressed) {
					enterPressed = true;
					recorder::active ? glue_stopRec() : glue_startRec();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == ' ' && !spacePressed) {
					spacePressed = true;
					G_Mixer.running ? glue_stopSeq() : glue_startSeq();
					ret = 1;
					break;
				}
			}
			else if (e == FL_KEYUP) {
				if (Fl::event_key() == FL_BackSpace)
					bckspcPressed = false;
				else if (Fl::event_key() == FL_End)
					endPressed = false;
				else if (Fl::event_key() == ' ')
					spacePressed = false;
				else if (Fl::event_key() == FL_Enter)
					enterPressed = false;
			}

			/* Walk button arrays, trying to match button's label with the Keyboard event.
			 * If found, set that button's value() based on up/down event,
			 * and invoke that button's callback() */

			for (int i=0; i<gChannelsL->children(); i++)
				ret &= keypress((gChannel*)gChannelsL->child(i), e);
			for (int i=0; i<gChannelsR->children(); i++)
				ret &= keypress((gChannel*)gChannelsR->child(i), e);

			break;
		}

		/*
		case FL_PUSH: {
			for (int t=0; t<children(); t++) {
				gChannel *ch = (gChannel*) child(t);
				if (ch->button->value()) {	      // if button ON do callback
					ch->button->do_callback();
					break;
				}
			}
			break;
		}
		*/

	}
	return ret;
}


/* ------------------------------------------------------------------ */


int Keyboard::keypress(gChannel *gch, int e) {
	if (e == FL_KEYDOWN && gch->button->value())	      // key already pressed! skip it
		return 1;
	else
	if (Fl::event_key() == gch->ch->key) {
		gch->button->take_focus();                        // move focus to this button
		gch->button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
		gch->button->do_callback();                       // invoke the button's callback
		return 1;                               				 // indicate we handled it
	}
	else
		return 0;

}


/* ------------------------------------------------------------------ */


void Keyboard::clear() {
	Fl::lock();
	gChannelsL->clear();
	gChannelsR->clear();
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		G_Mixer.channels.at(i)->guiChannel = NULL;
	Fl::unlock();

	gChannelsR->size(gChannelsR->w(), 0);
	gChannelsL->size(gChannelsL->w(), 0);

	addChannelL->position(gChannelsL->x(), gChannelsL->y()+gChannelsL->h());
	addChannelR->position(gChannelsR->x(), gChannelsR->y()+gChannelsR->h());

	redraw();
}


/* ------------------------------------------------------------------ */


void Keyboard::setChannelWithActions(channel *ch) {
	gChannel *gch = getChannel(ch);
	if (ch->hasActions) {
		ch->readActions = true;   /// <---- move this to glue_stopRec
		gch->addActionButton(true); // true = button on
	}
	else {
		ch->readActions = false;  /// <---- move this to glue_stopRec
		gch->remActionButton();
	}
}

