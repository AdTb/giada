/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_devInfo
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#include "../../core/kernelAudio.h"
#include "../../utils/gui.h"
#include "../../utils/string.h"
#include "../elems/ge_mixed.h"
#include "gd_devInfo.h"


extern KernelAudio G_KernelAudio;


using std::string;


gdDevInfo::gdDevInfo(unsigned dev)
	: Fl_Window(340, 300, "Device information")
{
	set_modal();

	text  = new gBox(8, 8, 320, 200, "", (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
	close = new gClick(252, h()-28, 80, 20, "Close");
	end();

	string body  = "";
	int    lines = 7;

	body  = "Device name: " + G_KernelAudio.getDeviceName(dev) + "\n";
	body += "Total output(s): " + gu_itoa(G_KernelAudio.getMaxOutChans(dev)) + "\n";
	body += "Total intput(s): " + gu_itoa(G_KernelAudio.getMaxInChans(dev)) + "\n";
	body += "Duplex channel(s): " + gu_itoa(G_KernelAudio.getDuplexChans(dev)) + "\n";
	body += "Default output: " + string(G_KernelAudio.isDefaultOut(dev) ? "yes" : "no") + "\n";
	body += "Default input: " + string(G_KernelAudio.isDefaultIn(dev) ? "yes" : "no") + "\n";

	int totalFreq = G_KernelAudio.getTotalFreqs(dev);
	body += "Supported frequencies: " + gu_itoa(totalFreq);

	for (int i=0; i<totalFreq; i++) {
		if (i % 6 == 0) {
			body += "\n    ";  // add new line each 6 printed freqs AND on the first line (i % 0 != 0)
			lines++;
		}
		body += gu_itoa( G_KernelAudio.getFreq(dev, i)) + "  ";
	}

	text->copy_label(body.c_str());

	/* resize the window to fit the content. fl_height() returns the height
	 * of a line. fl_height() * total lines + margins + button size */

	resize(x(), y(), w(), (lines * fl_height()) + 8 + 8 + 8 + 20);
	close->position(close->x(), (lines * fl_height()) + 8 + 8);

	close->callback(__cb_window_closer, (void*)this);
	gu_setFavicon(this);
	show();
}


gdDevInfo::~gdDevInfo() {}
