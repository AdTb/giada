
/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
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

#ifdef WITH_VST

#ifndef __GD_PLUGINWINDOW_GUI_H__
#define __GD_PLUGINWINDOW_GUI_H__


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "pluginHost.h"
#include "ge_mixed.h"

#if defined(__APPLE__)
	#include <Carbon/Carbon.h>
#endif

class gdPluginWindowGUI : public gWindow {
private:

	Plugin *pPlugin;
#if defined(__APPLE__)
	WindowRef window;
#endif

public:

	gdPluginWindowGUI(Plugin *pPlugin);
	~gdPluginWindowGUI();
};

#endif

#endif // #ifdef WITH_VST
