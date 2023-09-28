/*
  ==============================================================================

	LayoutViewer.cpp
	Created: 8 Oct 2020 6:59:51pm
	Author:  bkupe

  ==============================================================================
*/

#include "LayoutViewer.h"
#include "Brain.h"
#include "UserInputManager.h"

LayoutViewer::LayoutViewer() :
	ShapeShifterContentComponent("Layout Viewer")
{
	layoutsList.setTextWhenNoChoicesAvailable("No Layout created");
	layoutsList.setTextWhenNothingSelected("Select a Layout");
	layoutsList.addListener(this);
	addAndMakeVisible(&layoutsList);

	editMode.setButtonText("Edit mode");
	editMode.addListener(this);
	addAndMakeVisible(&editMode);

	viewPaths.setButtonText("Paths lines");
	viewPaths.addListener(this);
	addAndMakeVisible(&viewPaths);

	rebuildLayoutsList();
	LayoutManager::getInstance()->addAsyncManagerListener(this);

}

LayoutViewer::~LayoutViewer()
{
	if (selectedLayout != nullptr) {
		selectedLayout->removeChangeListener(this);
	}
	if (LayoutManager::getInstanceWithoutCreating()) LayoutManager::getInstance()->removeAsyncManagerListener(this);
}


void LayoutViewer::rebuildLayoutsList()
{
	int idToSelect = 0;

	layoutsList.clear(dontSendNotification);
	layoutsList.addItem("No Layout selected", -1);
	for (auto& l : LayoutManager::getInstance()->items)
	{
		if (l != nullptr) {
			int id = l->id->intValue();
			if (l == selectedLayout) idToSelect = id;
			layoutsList.addItem(l->niceName, id);
		}
	}

	layoutsList.setSelectedId(idToSelect, dontSendNotification);
}

void LayoutViewer::newMessage(const LayoutManager::ManagerEvent& e)
{
	if (e.type == LayoutManager::ManagerEvent::ITEM_ADDED || e.type == LayoutManager::ManagerEvent::ITEM_REMOVED || e.type == LayoutManager::ManagerEvent::ITEMS_REORDERED)
	{
		if (e.type == LayoutManager::ManagerEvent::ITEM_REMOVED && e.getItem() == selectedLayout) {
			selectedLayout = nullptr;
		}
		rebuildLayoutsList();
	}
}

Colour LayoutViewer::getClickColour(BKPath* path, ClicAction action)
{
	currentClicColour+= 5;
	uint32 temp = currentClicColour;
	int r = temp % 256;
	temp /= 256;
	int g = temp % 256;
	temp /= 256;
	int b = temp % 256;

	colourToPath.set(currentClicColour, path);
	colourToAction.set(currentClicColour, action);
	return Colour(r,g,b);
}

Colour LayoutViewer::getClickColour(SubFixture* sf)
{
	currentClicColour += 5;
	uint32 temp = currentClicColour;
	int r = temp % 256;
	temp /= 256;
	int g = temp % 256;
	temp /= 256;
	int b = temp % 256;

	colourToSubFixture.set(currentClicColour, sf);
	colourToAction.set(currentClicColour, CLIC_SELECT);
	return Colour(r, g, b);
}

Colour LayoutViewer::getClickColour(Fixture* f)
{
	currentClicColour += 5;
	uint32 temp = currentClicColour;
	int r = temp % 256;
	temp /= 256;
	int g = temp % 256;
	temp /= 256;
	int b = temp % 256;

	colourToFixture.set(currentClicColour, f);
	colourToAction.set(currentClicColour, CLIC_SELECT);
	return Colour(r, g, b);
}

void LayoutViewer::resetClickColour()
{
	currentClicColour = 0;
	colourToAction.clear();
	colourToAction.clear();
}

void LayoutViewer::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	int id = comboBoxThatHasChanged->getSelectedId();
	selectLayout(id);
}

void LayoutViewer::buttonStateChanged(Button* b)
{
	repaint();
	if (b == &editMode && selectedLayout != nullptr && editMode.getToggleState()) {
		selectedLayout->selectThis();
	}
}

void LayoutViewer::selectLayout(int id)
{
	if (selectedLayout != nullptr) {
		selectedLayout->removeChangeListener(this);
	}
	Layout* l = Brain::getInstance()->getLayoutById(id);
	if (l != nullptr) {
		selectedLayout = l;
		selectedLayout->addChangeListener(this);
		selectedLayout->computeData();
		if (editMode.getToggleState()) {
			selectedLayout->selectThis();
		}
	}
	else {
		selectedLayout = nullptr;
	}
	repaint();
}

void LayoutViewer::resized()
{
	Rectangle<int> r = getLocalBounds();
	Rectangle<int> hr = r.removeFromTop(20);

	layoutsList.setBounds(hr.removeFromLeft(80).reduced(2));
	viewPaths.setBounds(hr.removeFromLeft(80).reduced(2));
	editMode.setBounds(hr.removeFromLeft(80).reduced(2));
}


void LayoutViewer::mouseExit(const MouseEvent& e)
{
	currentMouseAction = CLIC_NOACTION;
	currentMousePath = nullptr;
	hoveredPath = nullptr;
}

void LayoutViewer::mouseDown(const MouseEvent& e)
{
	if (selectedLayout == nullptr) {return;}

	int x = e.getMouseDownX();
	int y = e.getMouseDownY();

	int r = clicZones.getPixelAt(x, y).getRed();
	int g = clicZones.getPixelAt(x, y).getGreen();
	int b = clicZones.getPixelAt(x, y).getBlue();
	
	int c = (((b*256) + g )*256) + r;

	if (colourToAction.contains(c)) {
		ClicAction a = colourToAction.getReference(c);

		if (a == CLIC_SELECT) {
			if (colourToFixture.contains(c)) {
				Fixture* f = colourToFixture.getReference(c);
				UserInputManager::getInstance()->gridViewCellPressed("fixture",f->id->intValue());
				//UserInputManager::getInstance()->processInput("Fixture");
				//UserInputManager::getInstance()->processInput(f->id->stringValue());
			}
			else if(colourToSubFixture.contains(c)) {
				SubFixture* sf = colourToSubFixture.getReference(c);
				UserInputManager::getInstance()->gridViewCellPressed("fixture", sf->parentFixture->id->intValue());
				if (sf->parentFixture->subFixtures.size() > 1) {
					UserInputManager::getInstance()->processInput("subfixture");
					UserInputManager::getInstance()->processInput(String(sf->subId));
				}
			}
 		}
		if (colourToPath.contains(c)) {

			float layoutX = jmap(float(e.getDistanceFromDragStartX() + e.getMouseDownX()), topLeftX, bottomRightX, (float)selectedLayout->dimensionsX->getValue()[0], (float)selectedLayout->dimensionsX->getValue()[1]);
			float layoutY = jmap(float(e.getDistanceFromDragStartY() + e.getMouseDownY()), topLeftY, bottomRightY, (float)selectedLayout->dimensionsY->getValue()[1], (float)selectedLayout->dimensionsY->getValue()[0]);

			currentMouseAction = colourToAction.getReference(c);
			BKPath* p = colourToPath.getReference(c);
			currentMousePath = p;
			deltaMouseX = (float)p->position->getValue()[0] - layoutX;
			deltaMouseY = (float)p->position->getValue()[1] - layoutY;
			if (currentMouseAction == CLIC_TL) { fixedMouseX = p->gridBR.x; fixedMouseY = p->gridBR.y; }
			if (currentMouseAction == CLIC_TR) { fixedMouseX = p->gridBL.x; fixedMouseY = p->gridBL.y; }
			if (currentMouseAction == CLIC_BL) { fixedMouseX = p->gridTR.x; fixedMouseY = p->gridTR.y; }
			if (currentMouseAction == CLIC_BR) { fixedMouseX = p->gridTL.x; fixedMouseY = p->gridTL.y; }

			if (e.getNumberOfClicks() == 2) {
				currentMousePath->selectThis();
			}
		}
	}
}

void LayoutViewer::mouseDrag(const MouseEvent& e)
{
	if (selectedLayout!= nullptr && currentMousePath != nullptr) {
		float layoutX = jmap(float(e.getDistanceFromDragStartX() + e.getMouseDownX()), topLeftX, bottomRightX, (float)selectedLayout->dimensionsX->getValue()[0], (float)selectedLayout->dimensionsX->getValue()[1]);
		float layoutY = jmap(float(e.getDistanceFromDragStartY() + e.getMouseDownY()), topLeftY, bottomRightY, (float)selectedLayout->dimensionsY->getValue()[1], (float)selectedLayout->dimensionsY->getValue()[0]);

		BKPath::PathType type = currentMousePath->pathType->getValueDataAsEnum<BKPath::PathType>();
		if (type == BKPath::PATH_POINT) {
			if (currentMouseAction == CLIC_DRAG) {
				var v;
				v.append(layoutX + deltaMouseX);
				v.append(layoutY + deltaMouseY);
				currentMousePath->position->setValue(v);
			}
		}
		else if (type == BKPath::PATH_LINE) {
			if (currentMouseAction == CLIC_DRAG) {
				float lineX = (float)currentMousePath->lineEndPosition->getValue()[0] - (float)currentMousePath->position->getValue()[0];
				float lineY = (float)currentMousePath->lineEndPosition->getValue()[1] - (float)currentMousePath->position->getValue()[1];

				var v;
				v.append(layoutX + deltaMouseX);
				v.append(layoutY + deltaMouseY);
				currentMousePath->position->setValue(v);
				v[0] = (float)v[0] + lineX;
				v[1] = (float)v[1] + lineY;
				currentMousePath->lineEndPosition->setValue(v);

			}
			else if (currentMouseAction == CLIC_ORIGIN) {
				var v;
				v.append(layoutX);
				v.append(layoutY);
				currentMousePath->position->setValue(v);
			}
			else if (currentMouseAction == CLIC_END) {
				var v;
				v.append(layoutX);
				v.append(layoutY);
				currentMousePath->lineEndPosition->setValue(v);
			}

		}
		else if (type == BKPath::PATH_ROD) {
			if (currentMouseAction == CLIC_DRAG) {
				float lineX = (float)currentMousePath->lineEndPosition->getValue()[0] - (float)currentMousePath->position->getValue()[0];
				float lineY = (float)currentMousePath->lineEndPosition->getValue()[1] - (float)currentMousePath->position->getValue()[1];

				var v;
				v.append(layoutX + deltaMouseX);
				v.append(layoutY + deltaMouseY);
				currentMousePath->position->setValue(v);

			}
			else if (currentMouseAction == CLIC_END) {
				Point<float> delta(layoutX, layoutY);
				delta.x -= (float)currentMousePath->position->getValue()[0];
				delta.y -= (float)currentMousePath->position->getValue()[1];

				currentMousePath->rodSize->setValue( delta.getDistanceFromOrigin() *2);
				currentMousePath->rodAngle->setValue(radiansToDegrees( BKPath::getVectAngle(&delta)));
			}

		}
		else if(type == BKPath::PATH_GRID) {
			if (currentMouseAction == CLIC_DRAG) {
				var v;
				v.append(layoutX + deltaMouseX);
				v.append(layoutY + deltaMouseY);
				currentMousePath->position->setValue(v);
			}
			else if (currentMouseAction == CLIC_TL || currentMouseAction == CLIC_TR || currentMouseAction == CLIC_BL || currentMouseAction == CLIC_BR) {
				float originX = fixedMouseX + ((layoutX - fixedMouseX) / 2);
				float originY = fixedMouseY + ((layoutY - fixedMouseY) / 2);

				Point<float> diag(layoutX-fixedMouseX, layoutY-fixedMouseY);
				Point<float> xAxis(1, 0);
				Point<float> yAxis(0, 1);
				BKPath::rotateVect(&xAxis, currentMousePath->gridAngle->floatValue());
				BKPath::rotateVect(&yAxis, currentMousePath->gridAngle->floatValue());
				float width = abs(diag.getDotProduct(xAxis));
				float height = abs(diag.getDotProduct(yAxis));

				var v;
				v.append(width);
				v.append(height);
				currentMousePath->gridSize->setValue(v);
				v[0] = originX;
				v[1] = originY;
				currentMousePath->position->setValue(v);
			}
			else if (currentMouseAction == CLIC_ROTATE) {
				Point<float> origin((float)currentMousePath->position->getValue()[0], (float)currentMousePath->position->getValue()[1]);
				Point<float> mouse(layoutX, layoutY);
				mouse -= origin;
				float angle = BKPath::getVectAngle(&mouse);
				currentMousePath->gridAngle->setValue(radiansToDegrees(angle));
			}
		}
		if (type == BKPath::PATH_CIRCLE) {
			if (currentMouseAction == CLIC_DRAG) {
				var v;
				v.append(layoutX + deltaMouseX);
				v.append(layoutY + deltaMouseY);
				currentMousePath->position->setValue(v);
			}
		}

	}
}

void LayoutViewer::mouseUp(const MouseEvent& e)
{
	currentMouseAction = CLIC_NOACTION;
	currentMousePath = nullptr;
}

void LayoutViewer::mouseMove(const MouseEvent& e)
{
	if (selectedLayout != nullptr && currentMousePath == nullptr) {
		BKPath* newHovered = nullptr;
		int x = e.getMouseDownX();
		int y = e.getMouseDownY();

		int r = clicZones.getPixelAt(x, y).getRed();
		int g = clicZones.getPixelAt(x, y).getGreen();
		int b = clicZones.getPixelAt(x, y).getBlue();

		int c = (((b * 256) + g) * 256) + r;

		if (colourToPath.contains(c) && editMode.getToggleState()) {
			newHovered = colourToPath.getReference(c);
		}
		else {
			newHovered = nullptr;
		}

		if (newHovered != hoveredPath) {
			hoveredPath = newHovered;
			repaint();
		}

	}
}

void LayoutViewer::changeListenerCallback(ChangeBroadcaster* source)
{
	repaint();
}

void LayoutViewer::drawMidArrow(Graphics& g, Point<float>& from, Point<float>& to)
{
	Point<float> endArrow;
	Line<float> line;
	endArrow = ((to - from)/2.0f);
	float arrSize = endArrow.getDistanceFromOrigin();
	float ratio = (arrSize + 4) / arrSize;
	endArrow *= ratio;
	endArrow += from;
	line.setStart(from);
	line.setEnd(endArrow);
	g.drawArrow(line, 0, 6, 8);
}

void LayoutViewer::drawMidArrow(Graphics& g, float fromX, float fromY, float toX, float toY)
{
	Point<float>from;
	from.setXY(fromX, fromY);
	Point<float>to;
	to.setXY(toX, toY);
	drawMidArrow(g, from, to);
}

void LayoutViewer::paint(Graphics& g)
{
	clicZones = Image(Image::ARGB, getWidth(), getHeight(), true);
	Graphics clicg = Graphics(clicZones);
	g.fillAll(Colour(0,0,0));
	clicg.fillAll(Colour(0.f, 0.f, 0.f, 0.f));
	resetClickColour();

	if (selectedLayout == nullptr) {
		g.setColour(Colours::white.withAlpha(.8f));
		g.setFont(jlimit<float>(12, 20, getHeight() - 30));
		g.drawText("No layout selected", getLocalBounds().toFloat(), Justification::centred, false);
		return;
	}

	var dimensionX = selectedLayout->dimensionsX->getValue();
	var dimensionY = selectedLayout->dimensionsY->getValue();
	if (dimensionX[1] < dimensionX[0]) { var temp = dimensionX[0]; dimensionX[0] = dimensionX[1]; dimensionX[1] = temp; }
	if (dimensionY[1] < dimensionY[0]) { var temp = dimensionY[0]; dimensionY[0] = dimensionY[1]; dimensionY[1] = temp; }

	float layoutWidth = (float)dimensionX[1] - (float)dimensionX[0];
	float layoutHeight = (float)dimensionY[1] - (float)dimensionY[0];
	if (layoutWidth == 0 || layoutHeight == 0) {
		return;
	}

	Rectangle<float> layoutZone;
	float layoutRatio = abs(layoutWidth / layoutHeight);
	float windowRatio = (float)getWidth() / ((float)getHeight()-20);
	float originX = 0;
	float originY = 0;
	float width = 0;
	float height = 0;
	float uiScale = 1;

	if (layoutRatio > windowRatio) {
		// crop top and bottom;
		originX = 0;
		width = getWidth();
		height = width / layoutRatio;
		originY = getHeight() - height;
		originY /= 2;
	}
	else {
		// crop left and right
		originY = 20;
		height = getHeight()-20;
		width = height * layoutRatio;
		originX = getWidth()-width;
		originX /= 2;
	}

	topLeftX = originX;
	topLeftY = originY;
	bottomRightX = originX + width;
	bottomRightY = originY + height;
	uiScale = width/layoutWidth;

	//g.setColour(Colour(64,64,64));
	//g.fillRect(originX, originY, width, height);
	g.reduceClipRegion(originX, originY, width, height);
	g.setOrigin(originX, originY+height);
	g.addTransform(AffineTransform::verticalFlip(1));
	g.fillAll(Colour(32, 32, 32));

	clicg.reduceClipRegion(originX, originY, width, height);
	clicg.setOrigin(originX, originY + height);
	clicg.addTransform(AffineTransform::verticalFlip(1));

	float fixtWidth = 5;
	float halfFixtWidth = fixtWidth/2;

	float handleWidth = 10;
	float halfHandleWidth = handleWidth/2;

	int currentClickColor = 1;
	colourToPath.clear();
	colourToSubFixture.clear();
	colourToFixture.clear();
	colourToAction.clear();

	for (int i = 0; i < selectedLayout->paths.items.size(); i++) {
		BKPath* p = selectedLayout->paths.items[i];
		BKPath::PathType type = p->pathType->getValueDataAsEnum<BKPath::PathType>(); //::PATH_LINE) 
		Colour hoverColour((uint8)255, (uint8)255, (uint8)255, (uint8)63);
		Colour handleColour((uint8)255, (uint8)255, (uint8)255);

		bool drawPaths = viewPaths.getToggleState();
		bool edit = editMode.getToggleState();

		if (type == BKPath::PATH_POINT) {
			float fromX = jmap((float)p->position->getValue()[0], (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
			float fromY = jmap((float)p->position->getValue()[1], (float)dimensionY[0], (float)dimensionY[1], (float)0, height);

			if (edit) {
				clicg.setColour(getClickColour(p, CLIC_DRAG));
				clicg.fillEllipse(fromX - halfHandleWidth, fromY - halfHandleWidth, handleWidth, handleWidth);
			}
			else {
				clicg.setColour(getClickColour(p, CLIC_SELECT));
				clicg.fillRect(fromX - halfFixtWidth, fromY - halfFixtWidth, fixtWidth, fixtWidth);
			}
			g.setColour(juce::Colours::orange);
			g.drawRect(fromX - halfFixtWidth, fromY - halfFixtWidth, fixtWidth, fixtWidth, (float)1);

			if (p == hoveredPath) {
				g.setColour(handleColour);
				g.fillEllipse(fromX - halfHandleWidth, fromY - halfHandleWidth, handleWidth, handleWidth);
			}

		}
		if (type == BKPath::PATH_LINE) {
			float fromX = jmap((float)p->position->getValue()[0], (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
			float fromY = jmap((float)p->position->getValue()[1], (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
			float toX = jmap((float)p->lineEndPosition->getValue()[0], (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
			float toY = jmap((float)p->lineEndPosition->getValue()[1], (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
			Line<float> line(Point<float>(fromX, fromY), Point<float>(toX, toY));

			if (drawPaths) { // wanna draw lines ?
				g.setColour(juce::Colours::lightgrey);
				g.drawLine(line, 0.5f);
			}

			if (edit) {
				clicg.setColour(getClickColour(p, CLIC_DRAG));
				clicg.drawLine(line, handleWidth);
				clicg.setColour(getClickColour(p, CLIC_ORIGIN));
				clicg.fillEllipse(fromX - halfHandleWidth, fromY - halfHandleWidth, handleWidth, handleWidth);
				clicg.setColour(getClickColour(p, CLIC_END));
				clicg.fillEllipse(toX - halfHandleWidth, toY - halfHandleWidth, handleWidth, handleWidth);
			}
			g.setColour(juce::Colours::orange);
			float currentArrowX = -1;
			float currentArrowY = -1;
			if (p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				for (int i = 0; i < p->selection.computedSelectedSubFixtures.size(); i++) {
					SubFixture* sf = p->selection.computedSelectedSubFixtures[i];
					if (p->subFixtToPos.contains(sf)) {
						std::shared_ptr<Point<float>> t = p->subFixtToPos.getReference(sf);
						float X = jmap((float)t->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
						float Y = jmap((float)t->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
						if (drawPaths && (currentArrowX != -1 || currentArrowY != -1)) {
							g.setColour(juce::Colours::lightgrey);
							drawMidArrow(g, currentArrowX, currentArrowY, X, Y);
						}
						currentArrowX = X;
						currentArrowY = Y;
						g.setColour(juce::Colours::orange);
						g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
						if (!edit) {
							clicg.setColour(getClickColour(sf));
							clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
						}
					}
				}
				p->isComputing.exit();
			}
			if (!p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				Array<Fixture*> drawedFixtures;
				for (int i = 0; i < p->selection.computedSelectedSubFixtures.size(); i++) {
					SubFixture* sf = p->selection.computedSelectedSubFixtures[i];
					if (p->fixtToPos.contains(sf->parentFixture) && !drawedFixtures.contains(sf->parentFixture)) {
						std::shared_ptr<Point<float>> t = p->fixtToPos.getReference(sf->parentFixture);
						float X = jmap((float)t->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
						float Y = jmap((float)t->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
						if (drawPaths && (currentArrowX != -1 || currentArrowY != -1)) {
							g.setColour(juce::Colours::lightgrey);
							drawMidArrow(g, currentArrowX, currentArrowY, X, Y);
						}
						currentArrowX = X;
						currentArrowY = Y;
						g.setColour(juce::Colours::orange);
						g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
						if (!edit) {
							clicg.setColour(getClickColour(sf->parentFixture));
							clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
						}
					}
				}
				p->isComputing.exit();
			}

			if (p == hoveredPath) {
				g.setColour(hoverColour);
				g.drawLine(line, halfHandleWidth);
				g.setColour(handleColour);
				g.fillEllipse(fromX - halfHandleWidth, fromY - halfHandleWidth, handleWidth, handleWidth);
				g.fillEllipse(toX - halfHandleWidth, toY - halfHandleWidth, handleWidth, handleWidth);
			}

		}
		if (type == BKPath::PATH_ROD) {
			Point tl(jmap(p->gridTL.x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width), jmap(p->gridTL.y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height));
			Point tr(jmap(p->gridTR.x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width), jmap(p->gridTR.y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height));
			Line<float> line(tl, tr);

			if (drawPaths) { // wanna draw lines ?
				g.setColour(juce::Colours::lightgrey);
				g.drawLine(line, 0.5f);
			}

			clicg.setColour(getClickColour(p, CLIC_DRAG));
			clicg.drawLine(line, handleWidth);
			clicg.setColour(getClickColour(p, CLIC_ORIGIN));
			clicg.fillEllipse(tl.x - halfHandleWidth, tl.y - halfHandleWidth, handleWidth, handleWidth);
			clicg.setColour(getClickColour(p, CLIC_END));
			clicg.fillEllipse(tr.x - halfHandleWidth, tr.y - halfHandleWidth, handleWidth, handleWidth);
			float currentArrowX = -1;
			float currentArrowY = -1;
			if (p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				for (int i = 0; i < p->selection.computedSelectedSubFixtures.size(); i++) {
					SubFixture* sf = p->selection.computedSelectedSubFixtures[i];
					if (p->subFixtToPos.contains(sf)) {
						std::shared_ptr<Point<float>> t = p->subFixtToPos.getReference(sf);
						float X = jmap((float)t->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
						float Y = jmap((float)t->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
						if (drawPaths && (currentArrowX != -1 || currentArrowY != -1)) {
							g.setColour(juce::Colours::lightgrey);
							drawMidArrow(g, currentArrowX, currentArrowY, X, Y);
						}
						currentArrowX = X;
						currentArrowY = Y;
						g.setColour(juce::Colours::orange);
						g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
						if (!edit) {
							clicg.setColour(getClickColour(sf));
							clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
						}
					}
				}
				p->isComputing.exit();
			}
			if (!p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				Array<Fixture*> drawedFixtures;
				for (int i = 0; i < p->selection.computedSelectedSubFixtures.size(); i++) {
					SubFixture* sf = p->selection.computedSelectedSubFixtures[i];
					if (p->fixtToPos.contains(sf->parentFixture) && !drawedFixtures.contains(sf->parentFixture)) {
						std::shared_ptr<Point<float>> t = p->fixtToPos.getReference(sf->parentFixture);
						float X = jmap((float)t->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
						float Y = jmap((float)t->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
						if (drawPaths && (currentArrowX != -1 || currentArrowY != -1)) {
							g.setColour(juce::Colours::lightgrey);
							drawMidArrow(g, currentArrowX, currentArrowY, X, Y);
						}
						currentArrowX = X;
						currentArrowY = Y;
						g.setColour(juce::Colours::orange);
						g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
						if (!edit) {
							clicg.setColour(getClickColour(sf->parentFixture));
							clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
						}
					}
				}
				p->isComputing.exit();
			}

			if (p == hoveredPath) {
				g.setColour(hoverColour);
				g.drawLine(line, halfHandleWidth);
				g.setColour(handleColour);
				g.fillEllipse(tl.x - halfHandleWidth, tl.y - halfHandleWidth, handleWidth, handleWidth);
				g.fillEllipse(tr.x - halfHandleWidth, tr.y - halfHandleWidth, handleWidth, handleWidth);
			}

		}
		if (type == BKPath::PATH_GRID) {
			if (drawPaths) {
				g.setColour(juce::Colours::lightgrey);
				for (int iGrid = 0; iGrid < p->gridPath.size() - 1; iGrid++) {
					float fromX = jmap(p->gridPath[iGrid]->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
					float fromY = jmap(p->gridPath[iGrid]->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
					float toX = jmap(p->gridPath[iGrid + 1]->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
					float toY = jmap(p->gridPath[iGrid + 1]->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
					Line<float> line(Point<float>(fromX, fromY), Point<float>(toX, toY));
					g.drawLine(line, 0.5f);
					drawMidArrow(g, fromX, fromY, toX, toY);
				}
			}

			Point tl (jmap(p->gridTL.x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width), jmap(p->gridTL.y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height));
			Point tr (jmap(p->gridTR.x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width), jmap(p->gridTR.y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height));
			Point bl (jmap(p->gridBL.x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width), jmap(p->gridBL.y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height));
			Point br (jmap(p->gridBR.x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width), jmap(p->gridBR.y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height));
			Point rotate(tr);
			rotate -= br;
			rotate /= 2;
			rotate += br;

			g.setColour(juce::Colours::orange);
			if (p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				for (auto it = p->subFixtToPos.begin(); it != p->subFixtToPos.end(); it.next()) {
					float X = jmap((float)it.getValue()->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
					float Y = jmap((float)it.getValue()->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
					g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
					if (!edit) {
						clicg.setColour(getClickColour(it.getKey()));
						clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
					}
				}
				p->isComputing.exit();
			}
			if (!p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				for (auto it = p->fixtToPos.begin(); it != p->fixtToPos.end(); it.next()) {
					float X = jmap((float)it.getValue()->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
					float Y = jmap((float)it.getValue()->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
					g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
					if (!edit) {
						clicg.setColour(getClickColour(it.getKey()));
						clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
					}
				}
				p->isComputing.exit();
			}

			if (edit) {
				Path path;
				path.startNewSubPath(tl);
				path.lineTo(tr);
				path.lineTo(br);
				path.lineTo(bl);
				path.closeSubPath();
				clicg.setColour(getClickColour(p, CLIC_DRAG));
				clicg.fillPath(path);

				clicg.setColour(getClickColour(p, CLIC_TL));
				clicg.fillEllipse(tl.x - halfHandleWidth, tl.y - halfHandleWidth, handleWidth, handleWidth);
				clicg.setColour(getClickColour(p, CLIC_TR));
				clicg.fillEllipse(tr.x - halfHandleWidth, tr.y - halfHandleWidth, handleWidth, handleWidth);
				clicg.setColour(getClickColour(p, CLIC_BL));
				clicg.fillEllipse(bl.x - halfHandleWidth, bl.y - halfHandleWidth, handleWidth, handleWidth);
				clicg.setColour(getClickColour(p, CLIC_BR));
				clicg.fillEllipse(br.x - halfHandleWidth, br.y - halfHandleWidth, handleWidth, handleWidth);
				clicg.setColour(getClickColour(p, CLIC_ROTATE));
				clicg.fillEllipse(rotate.x - halfHandleWidth, rotate.y - halfHandleWidth, handleWidth, handleWidth);

				if (p == hoveredPath) {
					g.setColour(hoverColour);
					g.fillPath(path);
					g.setColour(handleColour);
					g.fillEllipse(tl.x - halfHandleWidth, tl.y - halfHandleWidth, handleWidth, handleWidth);
					g.fillEllipse(tr.x - halfHandleWidth, tr.y - halfHandleWidth, handleWidth, handleWidth);
					g.fillEllipse(bl.x - halfHandleWidth, bl.y - halfHandleWidth, handleWidth, handleWidth);
					g.fillEllipse(br.x - halfHandleWidth, br.y - halfHandleWidth, handleWidth, handleWidth);
					g.fillEllipse(rotate.x - halfHandleWidth, rotate.y - halfHandleWidth, handleWidth, handleWidth);
				}

			}

		}
		if (type == BKPath::PATH_CIRCLE) {
			float from = -(p->circleFrom->floatValue() / 360.) * 2 * MathConstants<float>::pi;
			float to = -(p->circleTo->floatValue() / 360.) * 2 * MathConstants<float>::pi;
			from += MathConstants<float>::pi / 2;
			to += MathConstants<float>::pi / 2;
			float X = jmap(p->position->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
			float Y = jmap(p->position->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
			float W = p->circleRadius->floatValue() * uiScale;
			Path path;
			path.addArc(X - (W), Y - (W), W * 2, W * 2, from, to, true);
			if (drawPaths) {
				g.setColour(juce::Colours::lightgrey);
				g.strokePath(path, PathStrokeType(0.5));
			}
			if (edit) {
				clicg.setColour(getClickColour(p, CLIC_DRAG));
				clicg.strokePath(path, PathStrokeType(handleWidth));
			}
			if (p == hoveredPath) {
				g.setColour(hoverColour);
				g.strokePath(path, PathStrokeType(handleWidth));
			}

			g.setColour(juce::Colours::orange);
			if (p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				for (auto it = p->subFixtToPos.begin(); it != p->subFixtToPos.end(); it.next()) {
					float X = jmap((float)it.getValue()->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
					float Y = jmap((float)it.getValue()->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
					g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
					if (!edit) {
						clicg.setColour(getClickColour(it.getKey()));
						clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
					}
				}
				p->isComputing.exit();
			}
			if (!p->spreadSubFixtures->boolValue()) {
				p->isComputing.enter();
				for (auto it = p->fixtToPos.begin(); it != p->fixtToPos.end(); it.next()) {
					float X = jmap((float)it.getValue()->x, (float)dimensionX[0], (float)dimensionX[1], (float)0, width);
					float Y = jmap((float)it.getValue()->y, (float)dimensionY[0], (float)dimensionY[1], (float)0, height);
					g.drawRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth, (float)1);
					if (!edit) {
						clicg.setColour(getClickColour(it.getKey()));
						clicg.fillRect(X - halfFixtWidth, Y - halfFixtWidth, fixtWidth, fixtWidth);
					}
				}
				p->isComputing.exit();
			}
		}
	}
}

