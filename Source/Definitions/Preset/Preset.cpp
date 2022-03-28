/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Preset.h"
#include "PresetValue.h"
#include "../ChannelFamily/ChannelType/ChannelType.h"
#include "../../Brain.h"
#include "PresetManager.h"
#include "UI/GridView/PresetGridView.h"

Preset::Preset(var params) :
	BaseItem(params.getProperty("name", "Preset")),
	objectType(params.getProperty("type", "Preset").toString()),
	objectData(params),
	subFixtureValues("Subfixtures"),
	devTypeParam()
{
	saveAndLoadRecursiveData = true;
	editorIsCollapsed = true;
	nameCanBeChangedByUser = false;

	itemDataType = "Preset";
	id = addIntParameter("ID", "ID of this Preset", 1, 1);
	userName = addStringParameter("Name", "Name of this preset", "New preset");
	updateName();

	String presetExplain = "Behaviour of the preset : \n\n";
	presetExplain += "- SubFixture : preset applyes only to recorded SubFixtures\n\n";
	// presetExplain += "- SubFixture Type : preset applies only to SubFixtures who have the same SubFixture type (same SubFixture suffix in Fixture type)\n\n";
	presetExplain += "- Fixture type : preset applies only to SubFixtures from the same Fixture type\n\n";
	presetExplain += "- Same channels : preset applies to all SubFixtures with same channels\n\n";

	presetType = addEnumParameter("Type", presetExplain);
	presetType->addOption("SubFixture", 1);
	// presetType->addOption("SubFixture Type", 2);
	presetType->addOption("Fixture Type", 3);
	presetType->addOption("Same Channels type", 4);

	// to add a manager with defined data
	subFixtureValues.selectItemWhenCreated = false;
	addChildControllableContainer(&subFixtureValues);

	Brain::getInstance()->registerPreset(this, id->getValue());
	if (params.isVoid()) {
		subFixtureValues.addItem();
	}

}

Preset::~Preset()
{
	Brain::getInstance()->unregisterPreset(this);
}

void Preset::onContainerParameterChangedInternal(Parameter* p) {
	if (p == id) {
		Brain::getInstance()->registerPreset(this, id->getValue(), true);
	}
	if (p == userName || p == id) {
		updateName();
	}
	PresetGridView::getInstance()->updateCells();
}

void Preset::computeValues() {
	computedSubFixtureValues.clear();
	computedFixtureTypeValues.clear();
	computedSubFixtureTypeValues.clear();
	computedUniversalValues.clear();

	Array<PresetSubFixtureValues*> fixtValues = subFixtureValues.getItemsWithType<PresetSubFixtureValues>();
	int pType = presetType->getValue();

	for (int commandIndex = 0; commandIndex < fixtValues.size(); commandIndex++) {
		PresetSubFixtureValues* fixtVal = fixtValues[commandIndex];
		Fixture* fixt = nullptr;
		std::shared_ptr<SubFixture> subFixt = nullptr;
		FixtureType* type = nullptr;
		int fixtureId = fixtVal->targetFixtureId->getValue();
		int subFixtId = fixtVal->targetSubFixtureId->getValue();
		if (fixtureId > 0) {
			fixt = Brain::getInstance()->getFixtureById(fixtureId);
			if (fixt != nullptr) {
				type = dynamic_cast<FixtureType*>(fixt -> devTypeParam -> targetContainer.get());
				subFixt = fixt->subFixtures.getReference(subFixtId);
			}
		}
		
		Array<PresetValue *> values = fixtVal->values.getItemsWithType<PresetValue>();
		for (int valIndex = 0; valIndex < values.size(); valIndex++) {
			ChannelType* param = dynamic_cast<ChannelType*>(values[valIndex]->param->targetContainer.get());
			float value = values[valIndex] -> paramValue -> getValue();
			if (param != nullptr) {
				if (pType >= 1 && subFixt != nullptr) {
					HashMap<ChannelType*, float>* content = computedSubFixtureValues.getReference(subFixt.get());
					if (content == nullptr) {
						content = new HashMap<ChannelType*, float>();
						computedSubFixtureValues.set(subFixt.get(), content);
					}
					content -> set(param, value); 
				}
				// Fixture mode
				if (pType >= 3 && type != nullptr) {
					HashMap<ChannelType*, float>* content = computedFixtureTypeValues.getReference(type);
					if (content == nullptr) {
						content = new HashMap<ChannelType*, float>();
						computedFixtureTypeValues.set(type, content);
					}
					content->set(param, value);
				}

				// same param mode
				if (pType >= 4) {
					if (!computedUniversalValues.contains(param)) {
						computedUniversalValues.set(param, value);
					}
				}

			}

		}


	}

}

HashMap<ChannelType*, float>* Preset::getSubFixtureValues(std::shared_ptr<SubFixture> f) {
	HashMap<ChannelType*, float>* values = new HashMap<ChannelType*, float>();
	for (auto it = computedUniversalValues.begin(); it != computedUniversalValues.end(); it.next()) {
		values->set(it.getKey(), it.getValue());
	}

	FixtureType* dt = dynamic_cast<FixtureType*>(f->parentFixture->devTypeParam->targetContainer.get());
	if (computedFixtureTypeValues.contains(dt)) {
		HashMap<ChannelType*, float>* FixtureValues = computedFixtureTypeValues.getReference(dt);
		for (auto it = FixtureValues->begin(); it != FixtureValues->end(); it.next()) {
			values->set(it.getKey(), it.getValue());
		}
	}

	if (computedSubFixtureValues.contains(f.get())) {
		HashMap<ChannelType*, float>* FValues = computedSubFixtureValues.getReference(f.get());
		for (auto it = FValues->begin(); it != FValues->end(); it.next()) {
			values->set(it.getKey(), it.getValue());
		}
	}

	return values;
}


void Preset::updateName() {
	String n = userName->getValue();
	if (parentContainer != nullptr) {
		dynamic_cast<PresetManager*>(parentContainer.get())->reorderItems();
	}
	setNiceName(String((int)id->getValue()) + " - " + n);
}



void Preset::updateDisplay()
{
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

