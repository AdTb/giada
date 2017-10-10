/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * plugin
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

#ifndef G_PLUGIN_H
#define G_PLUGIN_H


#include "../deps/juce-config.h"


class Plugin
{
private:

	static int idGenerator;

	juce::AudioProcessorEditor* ui;    // gui
	juce::AudioPluginInstance* plugin; // core

	int id;
	bool bypass;

public:

	Plugin(juce::AudioPluginInstance* p, double samplerate, int buffersize);
	~Plugin();

	/* getUniqueId
	 * Return a string-based UID. */

	std::string getUniqueId() const;

	std::string getName() const;
	bool isEditorOpen() const;
	bool hasEditor() const;
	int getNumParameters() const;
	float getParameter(int index) const;
	std::string getParameterName(int index) const;
	std::string getParameterText(int index) const;
	std::string getParameterLabel(int index) const;
	bool isSuspended() const;
	bool isBypassed() const;
	void process(juce::AudioBuffer<float>& b, juce::MidiBuffer& m) const;
	int getNumPrograms() const;
	int getCurrentProgram() const;
	std::string getProgramName(int index) const;
	int getId() const;
	int getEditorW() const;
	int getEditorH() const;
	void setParameter(int index, float value) const;
	void prepareToPlay(double samplerate, int buffersize) const;
	void setCurrentProgram(int index) const;

	void showEditor(void* parent);

	/* closeEditor
	 * Shut down plugin GUI. */

	void closeEditor();

	void toggleBypass();
	void setBypass(bool b);

	/* midiInParams
	A list of midiIn hex values for parameter automation. */

	std::vector<uint32_t> midiInParams;
};

#endif

#endif // #ifdef WITH_VST
