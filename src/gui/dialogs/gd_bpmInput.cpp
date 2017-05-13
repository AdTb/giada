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


#include <cstring>
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/mixer.h"
#include "../../core/clock.h"
#include "../../glue/main.h"
#include "../../utils/gui.h"
#include "../elems/basics/button.h"
#include "../elems/basics/input.h"
#include "gd_bpmInput.h"
#include "gd_mainWindow.h"


extern gdMainWindow *mainWin;


using namespace giada::m;


gdBpmInput::gdBpmInput(const char *label)
: gdWindow(144, 36, "Bpm")
{
	if (conf::bpmX)
		resize(conf::bpmX, conf::bpmY, w(), h());

	set_modal();

	input_a = new geInput(8,  8, 30, 20);
	input_b = new geInput(42, 8, 20, 20);
	ok 		  = new geButton(66, 8, 70, 20, "Ok");
	end();

	char   a[4];
	snprintf(a, 4, "%d", (int) clock::getBpm());
	char   b[2];
	for (unsigned i=0; i<strlen(label); i++)	// looking for the dot
		if (label[i] == '.') {
			snprintf(b, 2, "%c", label[i+1]);
			break;
		}

	input_a->maximum_size(3);
	input_a->type(FL_INT_INPUT);
	input_a->value(a);
	input_b->maximum_size(1);
	input_b->type(FL_INT_INPUT);
	input_b->value(b);

	ok->shortcut(FL_Enter);
	ok->callback(cb_update_bpm, (void*)this);

	gu_setFavicon(this);
	setId(WID_BPM);
	show();
}


/* -------------------------------------------------------------------------- */


gdBpmInput::~gdBpmInput()
{
	conf::bpmX = x();
	conf::bpmY = y();
}


/* -------------------------------------------------------------------------- */


void gdBpmInput::cb_update_bpm(Fl_Widget *w, void *p) { ((gdBpmInput*)p)->__cb_update_bpm(); }


/* -------------------------------------------------------------------------- */


void gdBpmInput::__cb_update_bpm()
{
	if (strcmp(input_a->value(), "") == 0)
		return;
	glue_setBpm(input_a->value(), input_b->value());
	do_callback();
}
