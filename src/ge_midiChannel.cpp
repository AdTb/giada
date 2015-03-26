/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_midiChannel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include "ge_midiChannel.h"
#include "ge_channel.h"
#include "ge_sampleChannel.h"
#include "gd_mainWindow.h"
#include "gd_keyGrabber.h"
#include "gd_midiGrabber.h"
#include "gd_editor.h"
#include "gd_actionEditor.h"
#include "gd_warnings.h"
#include "gd_browser.h"
#include "gd_midiOutputSetup.h"
#include "gg_keyboard.h"
#include "pluginHost.h"
#include "mixer.h"
#include "conf.h"
#include "patch.h"
#include "graphics.h"
#include "channel.h"
#include "wave.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "glue.h"
#include "gui_utils.h"

#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gMidiChannel::gMidiChannel(int X, int Y, int W, int H, class MidiChannel *ch)
	: gChannel(X, Y, W, H, CHANNEL_MIDI), ch(ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 120; // (5 widgets * 20) + (5 paddings * 4)
#else
	int delta = 96; // (4 widgets * 20) + (4 paddings * 4)
#endif

	button       = new gButton (x(), y(), 20, 20);
	sampleButton = new gChannelButton(button->x()+button->w()+4, y(), w() - delta, 20, "-- MIDI --");
	mute         = new gClick (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx           = new gButton(solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial  (fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol          = new gDial  (solo->x()+solo->w()+4, y(), 20, 20);
#endif

	end();

  resizable(sampleButton);

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	sampleButton->callback(cb_openMenu, (void*)this);
	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::cb_button      (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_button(); }
void gMidiChannel::cb_mute        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_mute(); }
void gMidiChannel::cb_solo        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_solo(); }
void gMidiChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openMenu(); }
void gMidiChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_changeVol(); }
#ifdef WITH_VST
void gMidiChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openFxWindow(); }
#endif


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_mute()
{
	glue_setMute(ch);
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_solo()
{
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_changeVol()
{
	glue_setChanVol(ch, vol->value());
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void gMidiChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_button()
{
	if (button->value())
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_openMenu()
{
	Fl_Menu_Item rclick_menu[] = {
		{"Edit actions..."},                        // 0
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 1
			{"All"},                                  // 2
			{0},                                      // 3
		{"Setup MIDI output..."},                   // 4
		{"Setup MIDI input..."},                    // 5
		{"Delete channel"},                         // 6
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[1].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Delete channel") == 0) {
		if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
			return;
		glue_deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, GrabForInput), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, GrabForOutput), 0);
		return;
	}
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::refresh()
{
	if (ch->status == STATUS_OFF) {
		sampleButton->bgColor0 = COLOR_BG_0;
		sampleButton->bdColor  = COLOR_BD_0;
		sampleButton->txtColor = COLOR_TEXT_0;
	}
	else
	if (ch->status == STATUS_PLAY) {
		sampleButton->bgColor0 = COLOR_BG_2;
		sampleButton->bdColor  = COLOR_BD_1;
		sampleButton->txtColor = COLOR_TEXT_1;
	}
	else
	if (ch->status & (STATUS_WAIT | STATUS_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->recStatus & (REC_WAITING | REC_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	sampleButton->redraw();
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::reset()
{
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- MIDI --");
	sampleButton->redraw();
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::update()
{

	if (ch->midiOut) {
		char tmp[32];
		sprintf(tmp, "-- MIDI (channel %d) --", ch->midiOutChan+1);
		sampleButton->copy_label(tmp);
	}
	else
		sampleButton->label("-- MIDI --");

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::resize(int X, int Y, int W, int H)
{
  gChannel::resize(X, Y, W, H);

	/* this stuff makes sense only with FX button available. Do nothing
	 * otherwise */

#ifdef WITH_VST
	if (w() < BREAK_FX) {
		fx->hide();

		sampleButton->size(w() - (BREAK_DELTA - BREAK_UNIT), sampleButton->h());
	}
	else {
		fx->show();
		sampleButton->size(w() - BREAK_DELTA, sampleButton->h());
	}
	mute->resize(sampleButton->x()+sampleButton->w()+4, y(), 20, 20);
	solo->resize(mute->x()+mute->w()+4, y(), 20, 20);

	gChannel::init_sizes();
#endif
}


/* -------------------------------------------------------------------------- */


int gMidiChannel::keyPress(int e)
{
	return 1; // does nothing for MidiChannel
}
