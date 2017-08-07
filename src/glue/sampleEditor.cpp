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


#include <cassert>
#include <FL/Fl.H>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/sampleEditor.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/basics/button.h"
#include "../gui/elems/sampleEditor/waveTools.h"
#include "../gui/elems/sampleEditor/volumeTool.h"
#include "../gui/elems/sampleEditor/boostTool.h"
#include "../gui/elems/sampleEditor/panTool.h"
#include "../gui/elems/sampleEditor/pitchTool.h"
#include "../gui/elems/sampleEditor/rangeTool.h"
#include "../gui/elems/sampleEditor/waveform.h"
#include "../core/sampleChannel.h"
#include "../core/waveFx.h"
#include "../core/const.h"
#include "../utils/gui.h"
#include "channel.h"
#include "sampleEditor.h"


extern gdMainWindow *G_MainWin;


using namespace giada::m;


namespace giada {
namespace c     {
namespace sampleEditor
{
gdSampleEditor* getSampleEditorWindow()
{
	gdSampleEditor* se = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	assert(se != nullptr);
	return se;
}


/* -------------------------------------------------------------------------- */


void setBeginEndChannel(SampleChannel* ch, int b, int e)
{
	ch->setBegin(b);
	ch->setEnd(e);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	Fl::lock();
	gdEditor->rangeTool->refresh();
	Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void cut(SampleChannel* ch, int a, int b)
{
	if (!wfx::cut(ch->wave, a, b)) {
		gdAlert("Unable to cut the sample!");
		return;
	}
	setBeginEndChannel(ch, ch->getBegin(), ch->getEnd());
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->refresh();
	gdEditor->updateInfo();
}


/* -------------------------------------------------------------------------- */


void silence(SampleChannel* ch, int a, int b)
{
	wfx::silence(ch->wave, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void fade(SampleChannel* ch, int a, int b, int type)
{
	wfx::fade(ch->wave, a, b, type);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void smoothEdges(SampleChannel* ch, int a, int b)
{
	wfx::smooth(ch->wave, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void setStartEnd(SampleChannel* ch, int a, int b)
{
	setBeginEndChannel(ch, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->recalcPoints();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->redraw();
}


/* -------------------------------------------------------------------------- */


void trim(SampleChannel* ch, int a, int b)
{
	if (!wfx::trim(ch->wave, a, b)) {
		gdAlert("Unable to trim the sample!");
		return;
	}
	setBeginEndChannel(ch, ch->getBegin(), ch->getEnd());
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->refresh();
	gdEditor->updateInfo();
}


/* -------------------------------------------------------------------------- */


void setPlayHead(SampleChannel* ch, int f)
{
	ch->setTrackerPreview(f);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->redraw();
}


/* -------------------------------------------------------------------------- */


void setPreview(SampleChannel* ch, int mode)
{
	ch->setPreviewMode(mode);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->play->value(!gdEditor->play->value());
}


/* -------------------------------------------------------------------------- */


void rewindPreview(SampleChannel* ch)
{
	geWaveform* waveform = getSampleEditorWindow()->waveTools->waveform;
	if (waveform->isSelected() && ch->getTrackerPreview() != waveform->getSelectionA())
		setPlayHead(ch, waveform->getSelectionA());
	else
		setPlayHead(ch, 0);
}
}}}; // giada::c::sampleEditor::
