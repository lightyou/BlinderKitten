/*
  ==============================================================================

    CuelistAction.cpp
    Created: 3 Feb 2022 10:15:35pm
    Author:  No

  ==============================================================================
*/

#include "CuelistAction.h"
#include "../Cuelist/Cuelist.h"
#include "../../Brain.h"

CuelistAction::CuelistAction(var params) :
    Action(params)
{
    actionType = (ActionType)(int)params.getProperty("actionType", CL_GO);

    cuelistId = addIntParameter("Cuelist ID", "Id oth the target cuelist",0,0);
}

CuelistAction::~CuelistAction()
{
}

void CuelistAction::triggerInternal()
{
    Cuelist * target = Brain::getInstance()->getCuelistById(cuelistId->getValue());
    if (target == nullptr) return;

    switch (actionType)
    {
    case CL_GO:
        target->go();
        break;

    case CL_OFF:
        target->off();
        break;
    }
}

void CuelistAction::setValueInternal(var value) {
    Cuelist* target = Brain::getInstance()->getCuelistById(cuelistId->getValue());
    if (target == nullptr) return;

    float val = value;

    switch (actionType)
    {
    case CL_GO:
        if (val > 0) {
            target->go();
        }
        break;

    case CL_OFF:
        if (val > 0) {
            target->off();
        }
        break;

    case CL_HTPLEVEL:
        target->setHTPLevel(val);
        break;
    }
}