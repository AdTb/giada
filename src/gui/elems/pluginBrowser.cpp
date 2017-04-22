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


#ifdef WITH_VST


#include <FL/fl_draw.H>
#include "../../core/plugin.h"
#include "../../core/const.h"
#include "../../core/pluginHost.h"
#include "basics/boxtypes.h"
#include "pluginBrowser.h"


using std::vector;
using std::string;
using namespace giada;


gePluginBrowser::gePluginBrowser(int x, int y, int w, int h)
	: Fl_Browser(x, y, w, h)
{
	box(G_CUSTOM_BORDER_BOX);
	textsize(GUI_FONT_SIZE_BASE);
	textcolor(COLOR_TEXT_0);
	selection_color(COLOR_BG_1);
	color(COLOR_BG_0);

	this->scrollbar.color(COLOR_BG_0);
	this->scrollbar.selection_color(COLOR_BG_1);
	this->scrollbar.labelcolor(COLOR_BD_1);
	this->scrollbar.slider(G_CUSTOM_BORDER_BOX);

	this->hscrollbar.color(COLOR_BG_0);
	this->hscrollbar.selection_color(COLOR_BG_1);
	this->hscrollbar.labelcolor(COLOR_BD_1);
	this->hscrollbar.slider(G_CUSTOM_BORDER_BOX);

	type(FL_HOLD_BROWSER);

	computeWidths();

  column_widths(widths);
  column_char('\t');       // tabs as column delimiters

	refresh();

	end();
}


/* -------------------------------------------------------------------------- */


void gePluginBrowser::refresh()
{
	clear();

	add("NAME\tMANUFACTURER\tCATEGORY\tFORMAT\tUID");
	add("---\t---\t---\t---\t---");

	for (int i=0; i<pluginHost::countAvailablePlugins(); i++) {
		pluginHost::PluginInfo pi = pluginHost::getAvailablePluginInfo(i);
		string m = pluginHost::doesPluginExist(pi.uid) ? "" : "@-";
		string s = m + pi.name + "\t" + m + pi.manufacturerName + "\t" + m +
				pi.category +	"\t" + m + pi.format + "\t" + m + pi.uid;
		add(s.c_str());
	}

	for (unsigned i=0; i<pluginHost::countUnknownPlugins(); i++) {
		string s = "?\t?\t?\t?\t? " + pluginHost::getUnknownPluginInfo(i) + " ?";
		add(s.c_str());
	}
}


/* -------------------------------------------------------------------------- */


void gePluginBrowser::computeWidths()
{
	int w0, w1, w3;
	for (int i=0; i<pluginHost::countAvailablePlugins(); i++) {
		pluginHost::PluginInfo pi = pluginHost::getAvailablePluginInfo(i);
		w0 = (int) fl_width(pi.name.c_str());
		w1 = (int) fl_width(pi.manufacturerName.c_str());
		w3 = (int) fl_width(pi.format.c_str());
		if (w0 > widths[0]) widths[0] = w0;
		if (w1 > widths[1]) widths[1] = w1;
		if (w3 > widths[3]) widths[3] = w3;
	}
	widths[0] += 60;
	widths[1] += 60;
	widths[2] = fl_width("CATEGORY") + 60;
	widths[3] += 60;
	widths[4] = 0;
}


#endif
