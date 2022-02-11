/*
  ==============================================================================

    FixtureTypeChannel.cpp
    Created: 8 Nov 2021 7:28:28pm
    Author:  No

  ==============================================================================
*/

#include "EffectRow.h"
#include "../ChannelFamily/ChannelFamilyManager.h"
#include "../ChannelFamily/ChannelType/ChannelType.h"
#include "../SubFixture/SubFixture.h"
#include "../SubFixture/SubFixtureChannel.h"
#include "Effect.h"

EffectRow::EffectRow(var params) :
    BaseItem(params.getProperty("name", "EffectRow")),
    objectType(params.getProperty("type", "EffectRow").toString()),
    objectData(params),
    curveContainer("Curve"),
    paramContainer("Parameters"),
    curve("Curve")
{

    saveAndLoadRecursiveData = true;
    speed = curveContainer.addFloatParameter("Speed", "Effect of this Curve, relative to the effect speed", 1, 0);
    curvePresetOrValue = curveContainer.addEnumParameter("Curve Type", "Use a preset curve or draw it here ?");
    curvePresetOrValue->addOption("Drawed", "drawed");
    curvePresetOrValue->addOption("Preset", "preset");

    paramContainer.selectItemWhenCreated = false;

    curve.saveAndLoadRecursiveData = true;
	curve.editorIsCollapsed = true;
	curve.allowKeysOutside = false;
	curve.isSelectable = false;
	curve.length->setValue(1);
	curve.addKey(0, 0, false);
	curve.items[0]->easingType->setValueWithData(Easing::LINEAR);
	curve.addKey(1, 1, false);
	curve.selectItemWhenCreated = false;
	curve.editorCanBeCollapsed = true;
    curve.editorIsCollapsed = false;
    curveContainer.addChildControllableContainer(&curve);

    presetId = curveContainer.addIntParameter("Curve preset ID", "ID of the curve preset you want to use", 0, 0);
    curveOrigin = curveContainer.addFloatParameter("Curve Origin", "Y Value of the curve to use as origin", 0, 0, 1);

    curveContainer.saveAndLoadRecursiveData = true;
    paramContainer.saveAndLoadRecursiveData = true;

    addChildControllableContainer(&curveContainer);
    addChildControllableContainer(&selection);
    addChildControllableContainer(&paramContainer);
    
    updateDisplay();
};

EffectRow::~EffectRow()
{
};


void EffectRow::computeData() {
    computedPositions.clear();
    selection.computeSelection();
    Effect* parentEffect = dynamic_cast<Effect*>(parentContainer->parentContainer.get());
    if (parentEffect == nullptr) {return;}
    for (int i = 0; i < selection.computedSelectedSubFixtures.size(); i++) {
        double deltaPos = 0;
        computedPositions.set(selection.computedSelectedSubFixtures[i], 0);
    }
    for (int i = 0; i < paramContainer.items.size(); i++) {
        EffectParam* p = paramContainer.items[i];
        p -> SubFixtureChannelOffsets.clear();
        ChannelType* chanType = dynamic_cast<ChannelType*>(p->paramType->targetContainer.get());
        if (chanType != nullptr) {
            Array<SubFixtureChannel*> chans;
            for (int fIndex = 0; fIndex < selection.computedSelectedSubFixtures.size(); fIndex++) {
                SubFixture* f = selection.computedSelectedSubFixtures[fIndex];
                SubFixtureChannel* c = f->channelsMap.getReference(chanType);
                if (c != nullptr) {
                    chans.add(c);
                }
            }
            int totWings= p->wings->getValue();
            int sizeWing = chans.size()/totWings;
            int nBuddying = p->buddying->getValue();
            float nLimit = chans.size();
            nLimit /= totWings;
            for (int chanIndex = 0; chanIndex < chans.size(); chanIndex++) {
                int nWing = chanIndex/sizeWing;
                double offset = (chanIndex - (chanIndex%nBuddying)) / nLimit;
                if (nWing % 2 == 1) {
                    offset = -offset;
                }
                offset *= (double)p->elementsSpread->getValue();
                offset += (double)p->elementsStart->getValue();
                p->SubFixtureChannelOffsets.set(chans[chanIndex], -offset);
                if (!parentEffect->chanToFxParam.contains(chans[chanIndex])) {
                    parentEffect->chanToFxParam.set(chans[chanIndex], new Array<EffectParam*>());
                }
                parentEffect->chanToFxParam.getReference(chans[chanIndex])->add(p);
            }
        }
    }
}

void EffectRow::onControllableFeedbackUpdate( ControllableContainer* cc, Controllable* c) {
    if (c == curvePresetOrValue) {
        updateDisplay();
    }
}

void EffectRow::updateDisplay() {
    bool curveIsPreset = curvePresetOrValue->getValue() == "preset";
    presetId->hideInEditor = !curveIsPreset;
    curve.hideInEditor = curveIsPreset;

    queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

