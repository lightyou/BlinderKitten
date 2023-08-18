/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Layout.h"
#include "LayoutManager.h"
#include "BKPath/BKPath.h"
#include "Definitions/Fixture/Fixture.h"
#include "Definitions/SubFixture/SubFixture.h"
#include "Brain.h"
#include "UI/LayoutViewer.h"
#include "UserInputManager.h"

Layout::Layout(var params) :
	BaseItem(params.getProperty("name", "Layout")),
	objectType(params.getProperty("Layouts", "Layout").toString()),
	// parameters("Parameters"),
	paths("Paths"),
	objectData(params)
	// previousID(-1),
	// slideManipParameter(nullptr)
{
	saveAndLoadRecursiveData = true;
	
	editorIsCollapsed = true;

	itemDataType = "Layout";
	canBeDisabled = false;

	// definitions->addBaseManagerListener(this);

	id = addIntParameter("ID", "ID of this layer", 1, 1);
	userName = addStringParameter("Name", "Name of this layer", "New layer");
	updateName();

	dimensionsX = addPoint2DParameter("From To X", "");
	var dx = -10; dx.append(10);
	dimensionsX->setDefaultValue(dx);
	dimensionsY = addPoint2DParameter("From To Y", "");
	var dy = -10; dy.append(10);
	dimensionsY->setDefaultValue(dy);

	addChildControllableContainer(&paths);
	paths.selectItemWhenCreated = false;

	var objectsData = params.getProperty("objects", var());
	Brain::getInstance()->registerLayout(this, id->getValue());

}

Layout::~Layout()
{
	Brain::getInstance()->unregisterLayout(this);
}

void Layout::updateName()
{
	String n = userName->getValue();
	if (parentContainer != nullptr) {
		dynamic_cast<LayoutManager*>(parentContainer.get())->reorderItems();
	}
	setNiceName(String((int)id->getValue()) + " - " + n);
}

void Layout::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == userName || p == id) {
		updateName();
	}
	sendChangeMessage();
	
}

void Layout::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	computeData();
	sendChangeMessage();
}

void Layout::computeData()
{
	isComputing.enter();

	subFixtToPos.clear();
	fixtToPos.clear();
	for (int iPath = 0; iPath < paths.items.size(); iPath++) {
		BKPath* p = paths.items[iPath];
		p->computeData();
		for (auto it = p->subFixtToPos.begin(); it != subFixtToPos.end(); it.next()) {
			subFixtToPos.set(it.getKey(), it.getValue());
		}
	}

	isComputing.exit();
}

std::shared_ptr<HashMap<SubFixture*, float>> Layout::getSubfixturesRatioFromDirection(float angle)
{
	std::shared_ptr<HashMap<SubFixture*, float>> ret = std::make_shared<HashMap<SubFixture*, float>>();
	Vector3D<float> xAxis(1,0,0);
	BKPath::rotateVect(&xAxis, angle);

	float minDot = (float)INT32_MAX;
	float maxDot = -(float)INT32_MAX;
	computeData();
	isComputing.enter();
	for (auto it = subFixtToPos.begin(); it != subFixtToPos.end(); it.next()) {
		Vector3D<float> sfAxis(it.getValue()->x, it.getValue()->y, 0);
		float dot = xAxis*sfAxis;
		minDot = jmin(minDot, dot);
		maxDot = jmax(maxDot, dot);
		ret->set(it.getKey(), dot);
	}
	isComputing.exit();
	for (auto it = ret->begin(); it != ret->end(); it.next()) {
		ret->set(it.getKey(), jmap(it.getValue(), minDot, maxDot, (float)0, (float)1));
	}
	return ret;
}


