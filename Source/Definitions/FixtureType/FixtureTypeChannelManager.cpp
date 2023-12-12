#include "FixtureTypeChannelManager.h"
#include "../../Brain.h"

FixtureTypeChannelManager::FixtureTypeChannelManager() :
    BaseManager("Channels")
{
    itemDataType = "FixtureTypeChannel";
    selectItemWhenCreated = false;       
}

FixtureTypeChannelManager::~FixtureTypeChannelManager()
{
}

void FixtureTypeChannelManager::calcDmxChannels() {
	int current = 1;
	for (int i = 0; i < items.size(); i++) {
		items[i]->setNiceName("temp "+String(i));
	}
	for (int i = 0; i < items.size(); i++) {
		items[i]->dmxDelta->setValue(current);
		items[i]->setNiceName("Channel "+String(current));
		if (items[i]->resolution->getValue() == "8bits") {
			current += 1;
		}
		else if (items[i]->resolution->getValue() == "16bits") {
			current += 2;
		}
		else {
			LOG("no resolution !");
		}
	}
}



/*
void FixtureTypeChannelManager::askForMoveBefore(BaseItem* c) {
    BaseManager::askForMoveBefore(c);
};
void FixtureTypeChannelManager::askForMoveAfter(BaseItem* c) {
    BaseManager::askForMoveAfter(c);
};
void FixtureTypeChannelManager::addItemInternal(FixtureTypeChannel* c, var data) {
};

void FixtureTypeChannelManager::askForMoveAfter(BaseItem* c) {
    BaseManager::askForMoveAfter(c);
};
void FixtureTypeChannelManager::askForMoveAfter(BaseItem* c) {
    BaseManager::askForMoveAfter(c);
};
*/

void FixtureTypeChannelManager::addItemInternal(FixtureTypeChannel* c, var data) {
	calcDmxChannels();
	if (parentContainer != nullptr && !Brain::getInstance()->loadingIsRunning) {
		FixtureType* ft = dynamic_cast<FixtureType* >(parentContainer.get());
		ft->updateVirtualLists();
	}

}
void FixtureTypeChannelManager::askForRemoveBaseItem(BaseItem* item) {
	BaseManager::askForRemoveBaseItem(item);
	calcDmxChannels();
}
void FixtureTypeChannelManager::askForDuplicateItem(BaseItem* item) {
	BaseManager::askForDuplicateItem(item);
	calcDmxChannels();
}
void FixtureTypeChannelManager::askForPaste() {
	BaseManager::askForPaste();
	calcDmxChannels();
}
void FixtureTypeChannelManager::askForMoveBefore(BaseItem* i) {
	BaseManager::askForMoveBefore(i);
	calcDmxChannels();
}
void FixtureTypeChannelManager::askForMoveAfter(BaseItem* i) {
	BaseManager::askForMoveAfter(i);
	calcDmxChannels();
}
