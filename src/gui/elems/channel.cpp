/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channel
 *
 * -----------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------- */


#include "../../core/mixer.h"
#include "../../core/conf.h"
#include "../../core/patch_DEPR_.h"
#include "../../core/graphics.h"
#include "../../utils/gui.h"
#include "../../glue/channel.h"
#include "../dialogs/gd_mainWindow.h"
#include "ge_column.h"
#include "channelButton.h"
#include "channel.h"


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch_DEPR_   G_Patch_DEPR_;
extern gdMainWindow *mainWin;


geChannel::geChannel(int X, int Y, int W, int H, int type)
 : Fl_Group(X, Y, W, H, NULL),
   type    (type)
{
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_arm(Fl_Widget *v, void *p) { ((geChannel*)p)->__cb_arm(); }


/* -------------------------------------------------------------------------- */


void geChannel::__cb_arm()
{
  // TODO 
  // glue_toggleArm(ch, true);
}


/* -------------------------------------------------------------------------- */


int geChannel::getColumnIndex()
{
	return ((gColumn*)parent())->getIndex();
}


/* -------------------------------------------------------------------------- */


void geChannel::blink()
{
	if (gu_getBlinker() > 6)
		mainButton->setPlayMode();
	else
    mainButton->setDefaultMode();
}


/* -------------------------------------------------------------------------- */


void geChannel::setColorsByStatus(int playStatus, int recStatus)
{
  switch (playStatus) {
    case STATUS_OFF:
  		mainButton->setDefaultMode();
      button->imgOn  = channelPlay_xpm;
      button->imgOff = channelStop_xpm;
      button->redraw();
      break;
    case STATUS_PLAY:
      mainButton->setPlayMode();
      button->imgOn  = channelStop_xpm;
      button->imgOff = channelPlay_xpm;
      button->redraw();
      break;
    case STATUS_WAIT:
      blink();
      break;
    case STATUS_ENDING:
      mainButton->setEndingMode();
      break;
  }

  switch (recStatus) {
    case REC_WAITING:
      blink();
      break;
    case REC_ENDING:
      mainButton->setEndingMode();
      break;
  }
}


/* -------------------------------------------------------------------------- */


void geChannel::packWidgets()
{
  /* Count visible widgets and resize mainButton according to how many widgets
  are visible. */

  int visibles = 0;
  for (int i=0; i<children(); i++) {
    child(i)->size(20, 20);  // also normalize widths
    if (child(i)->visible())
      visibles++;
  }
  mainButton->size(w() - ((visibles - 1) * (24)), 20);  // -1: exclude itself

  /* Reposition everything else */

  for (int i=1, p=0; i<children(); i++) {
    if (!child(i)->visible())
      continue;
    for (int k=i-1; k>=0; k--) // Get the first visible item prior to i
      if (child(k)->visible()) {
        p = k;
        break;
      }
    child(i)->position(child(p)->x() + child(p)->w() + 4, y());
  }

  init_sizes(); // Resets the internal array of widget sizes and positions
}


/* -------------------------------------------------------------------------- */


int geChannel::handleKey(int e, int key)
{
	int ret;
	if (e == FL_KEYDOWN && button->value())                              // key already pressed! skip it
		ret = 1;
	else
	if (Fl::event_key() == key && !button->value()) {
		button->take_focus();                                              // move focus to this button
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
		button->do_callback();                                             // invoke the button's callback
		ret = 1;
	}
	else
		ret = 0;

	if (Fl::event_key() == key)
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state

	return ret;
}
