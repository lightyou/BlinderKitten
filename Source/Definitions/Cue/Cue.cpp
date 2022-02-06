/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Cue.h"
#include "../Cuelist/Cuelist.h"
#include "../../Brain.h"

Cue::Cue(var params) :
	BaseItem(params.getProperty("name", "Cue")),
	objectType(params.getProperty("type", "Cue").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	editorIsCollapsed = false;
	itemDataType = "Cue";
	
	id = addFloatParameter("ID", "Id of this cue", 0, 0);
	
	autoFollow = addEnumParameter("Auto Follow", "Does the cuelist stops the execution of the cuelist or auto triggers the next one");
	autoFollow->addOption("Wait for go", "none");
	autoFollow->addOption("End of transitions", "auto");
	autoFollow->addOption("Immediate", "immediate");
	autoFollowTiming = addFloatParameter("Auto Follow timing", "Number of seconds before trigger the auto go ", 0, 0);
	autoFollowCountDown = addFloatParameter("Auto Follow CountdDown", "Triggers next cue when arrives to 0", 0, 0);
	autoFollowCountDown->isControllableFeedbackOnly = true;

	goBtn = addTrigger("GO", "trigger this cue");

	BaseManager<Command>* m = new BaseManager<Command>("Commands");
	m->selectItemWhenCreated = false;
	commands.reset(m);
	addChildControllableContainer(commands.get());


}

Cue::~Cue()
{
	LOG("delete cue");
}


void Cue::triggerTriggered(Trigger* t) {
	if (t == goBtn) {
		Cuelist* parentCuelist = dynamic_cast<Cuelist*>(this->parentContainer->parentContainer.get());
		Cue* temp = this;
		parentCuelist->go(temp);
	}
}

void Cue::onContainerParameterChangedInternal(Parameter* p) {
	if (p == autoFollowTiming) {
		float v = p->getValue();
		v = jmax((float)1, v);
		autoFollowCountDown->setRange(0,v);
	}
}

void Cue::computeValues() {
	maxTiming = 0;
	computedValues.clear();
	Array<Command*> cs = commands->getItemsWithType<Command>();
	for (int i = 0; i < cs.size(); i++) {
		cs[i]->computeValues();
		maxTiming = std::max(maxTiming, cs[i]->maxTiming);
		for (auto it = cs[i]->computedValues.begin(); it != cs[i]->computedValues.end(); it.next()) {
			FixtureChannel* fc = it.getKey();
			computedValues.set(fc, it.getValue());
		}
	}
}

void Cue::go() {
	if (autoFollow->getValue() == "immediate") {
		int64 now = Time::getMillisecondCounter();
		TSAutoFollowStart = now;
		float delay = autoFollowTiming->getValue();
		TSAutoFollowEnd = now + (1000*delay);
		Brain::getInstance()->pleaseUpdate(this);
	}
}

void Cue::update(int64 now) {
	if (TSAutoFollowEnd != 0 && now < TSAutoFollowEnd) {
		float remaining = TSAutoFollowEnd-now;
		remaining /=  1000;
		autoFollowCountDown->setValue(remaining);
		Brain::getInstance()->pleaseUpdate(this);
	}
	else {
		TSAutoFollowEnd = 0;
		autoFollowCountDown->setValue(0);
		Cuelist* parentCuelist = dynamic_cast<Cuelist*>(this->parentContainer->parentContainer.get());
		parentCuelist->go();
	}
}

void Cue::endTransition() {
	if (autoFollow->getValue() == "auto") {
		int64 now = Time::getMillisecondCounter();
		TSAutoFollowStart = now;
		float delay = autoFollowTiming->getValue();
		TSAutoFollowEnd = now + (1000 * delay);
		Brain::getInstance()->pleaseUpdate(this);
	}
}