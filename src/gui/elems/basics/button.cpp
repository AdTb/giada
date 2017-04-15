/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geButton
 * A regular button.
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


#include <FL/fl_draw.H>
#include "../../../core/const.h"
#include "button.h"


geButton::geButton(int x, int y, int w, int h, const char *L,
  const char **imgOff, const char **imgOn)
: geBaseButton(x, y, w, h, L),
  imgOff      (imgOff),
  imgOn       (imgOn),
  bgColor0    (COLOR_BG_0),
  bgColor1    (COLOR_BG_1),
  bdColor     (COLOR_BD_0),
  txtColor    (COLOR_TEXT_0)
{
}


/* -------------------------------------------------------------------------- */


void geButton::draw()
{
  if (!active()) txtColor = bdColor;
  else           txtColor = COLOR_TEXT_0;

  fl_rect(x(), y(), w(), h(), bdColor);             // borders
  if (value()) {                                    // -- clicked
    if (imgOn != nullptr)
      fl_draw_pixmap(imgOn, x()+1, y()+1);
    else
      fl_rectf(x(), y(), w(), h(), bgColor1);       // covers the border
  }
  else {                                            // -- not clicked
    fl_rectf(x()+1, y()+1, w()-2, h()-2, bgColor0); // bg inside the border
    if (imgOff != nullptr)
      fl_draw_pixmap(imgOff, x()+1, y()+1);
  }
  if (!active())
    fl_color(FL_INACTIVE_COLOR);

  fl_color(txtColor);
  fl_font(FL_HELVETICA, GUI_FONT_SIZE_BASE);
  fl_draw(label(), x()+2, y(), w()-2, h(), FL_ALIGN_CENTER);
}
