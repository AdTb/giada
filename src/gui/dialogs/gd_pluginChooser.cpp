/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginChooser
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


#ifdef WITH_VST


#include "../../utils/gui_utils.h"
#include "../../core/channel.h"
#include "../../core/pluginHost.h"
#include "../elems/ge_pluginBrowser.h"
#include "gd_pluginChooser.h"


extern PluginHost G_PluginHost;


gdPluginChooser::gdPluginChooser(int X, int Y, int W, int H)
  : gWindow(X, W, W, H, "Available plugins")
{
  browser = new gePluginBrowser(8, 8, w()-16, 200);
  resizable(browser);
  end();
	gu_setFavicon(this);
  show();
}

#endif // #ifdef WITH_VST
