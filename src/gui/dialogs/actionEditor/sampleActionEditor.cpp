/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "../../../core/const.h"
#include "../../../core/midiEvent.h"
#include "../../../core/graphics.h"
#include "../../../core/sampleChannel.h"
#include "../../../glue/actionEditor.h"
#include "../../elems/basics/scroll.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/resizerBar.h"
#include "../../elems/basics/choice.h"
#include "../../elems/basics/box.h"
#include "../../elems/actionEditor/sampleActionEditor.h"
#include "../../elems/actionEditor/envelopeEditor.h"
#include "../../elems/actionEditor/gridTool.h"
#include "sampleActionEditor.h"


using std::string;


namespace giada {
namespace v
{
gdSampleActionEditor::gdSampleActionEditor(m::SampleChannel* ch)
: gdBaseActionEditor(ch)
{
	computeWidth();

	/* Container with zoom buttons and the action type selector. Scheme of the 
	resizable boxes: |[--b1--][actionType][--b2--][+][-]| */

	Fl_Group* upperArea = new Fl_Group(8, 8, w()-16, 20);

	upperArea->begin();

	  actionType = new geChoice(8, 8, 80, 20);
	  gridTool   = new geGridTool(actionType->x()+actionType->w()+4, 8);
		actionType->add("Key press");
		actionType->add("Key release");
		actionType->add("Kill chan");
		actionType->value(0);

		if (!canChangeActionType())
			actionType->deactivate();

		geBox* b1  = new geBox(gridTool->x()+gridTool->w()+4, 8, 300, 20);    // padding actionType - zoomButtons
		zoomInBtn  = new geButton(w()-8-40-4, 8, 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
		zoomOutBtn = new geButton(w()-8-20,   8, 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);

	upperArea->end();
	upperArea->resizable(b1);

	zoomInBtn->callback(cb_zoomIn, (void*)this);
	zoomOutBtn->callback(cb_zoomOut, (void*)this);

	/* Main viewport: contains all widgets. */

	viewport = new geScroll(8, 36, w()-16, h()-44);

	m_ae  = new geSampleActionEditor(viewport->x(), viewport->y(), ch);
	m_aer = new geResizerBar(m_ae->x(), m_ae->y()+m_ae->h(), viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H);
	viewport->add(m_ae);
	viewport->add(m_aer);
	
	m_ee  = new geEnvelopeEditor(viewport->x(), m_ae->y()+m_ae->h()+RESIZER_BAR_H, "volume", ch);
	m_eer = new geResizerBar(m_ee->x(), m_ee->y()+m_ee->h(), viewport->w(), RESIZER_BAR_H, MIN_WIDGET_H);
	viewport->add(m_ee);
	viewport->add(m_eer);

	end();
	prepareWindow();
	rebuild();
}


/* -------------------------------------------------------------------------- */


bool gdSampleActionEditor::canChangeActionType()
{
	m::SampleChannel* sch = static_cast<m::SampleChannel*>(ch); 
	return sch->mode != ChannelMode::SINGLE_PRESS && !sch->isAnyLoopMode();
}


/* -------------------------------------------------------------------------- */


void gdSampleActionEditor::rebuild()
{
	m_actions = c::actionEditor::getActions(ch);
	canChangeActionType() ? actionType->activate() : actionType->deactivate(); 
	computeWidth();
	m_ae->rebuild();
	m_aer->size(m_ae->w(), m_aer->h());
	m_ee->rebuild();	
	m_eer->size(m_ee->w(), m_eer->h());
}
}} // giada::v::
