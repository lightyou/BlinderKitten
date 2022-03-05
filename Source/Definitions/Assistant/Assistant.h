/*
  ==============================================================================

    Assistant.h
    Created: 29 Jan 2019 3:52:46pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Definitions/Command/CommandValueManager.h"

class Assistant :
	public BaseItem,
	Thread
{
public:
	juce_DeclareSingleton(Assistant, true);

	Assistant();
	~Assistant();

	void run() override;

	ControllableContainer paletteMakerCC;
	IntParameter* paletteGroupId;
	IntParameter* paletteFirstPresetId;
	IntParameter* paletteLastPresetId;
	IntParameter* paletteTimingPresetId;
	Trigger* paletteBtn;

	ControllableContainer masterMakerCC;
	IntParameter* masterFirstGroupId;
	IntParameter* masterLastGroupId;
	CommandValueManager masterValue;
	Trigger* masterBtn;

	bool pleaseCreatePalette = false;
	bool pleaseCreateMasters = false;

	void triggerTriggered(Trigger* t);
	void onContainerParameterChangedInternal(Parameter* p);
	void updateDisplay();
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c);

	void createPalette();
	void createMasters();

};