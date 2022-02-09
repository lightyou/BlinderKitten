/*
  ==============================================================================

    Brain.h
    Created: 14 Nov 2021 10:42:56pm
    Author:  No

  ==============================================================================
*/
#pragma once

#include "JuceHeader.h"
#include "Definitions/Fixture/Fixture.h"
#include "Definitions/Device/Device.h"
#include "Definitions/Group/Group.h"
#include "Definitions/Command/Command.h"
#include "Definitions/Cue/Cue.h"
#include "Definitions/Cuelist/Cuelist.h"
#include "Definitions/Preset/Preset.h"
#include "Definitions/Programmer/Programmer.h"
#include "Definitions/CurvePreset/CurvePreset.h"
#include "Definitions/TimingPreset/TimingPreset.h"
#include "Definitions/Effect/Effect.h"

class Brain :
    public Thread
{
public:
    juce_DeclareSingleton(Brain, true);
    Brain();
    ~Brain();

    HashMap<int, Fixture*>fixtures;
    HashMap<int, Group*>groups;
    HashMap<int, Device*>devices;
    HashMap<int, Preset*>presets;
    HashMap<int, Cuelist*>cuelists;
    HashMap<int, Programmer*>programmers;
    HashMap<int, CurvePreset*>curvePresets;
    HashMap<int, TimingPreset*>timingPresets;
    HashMap<int, Effect*>effects;

    Array<Cuelist*> cuelistPoolUpdating;
    Array<Cuelist*> cuelistPoolWaiting;
    Array<FixtureChannel*> fixtureChannelPoolUpdating;
    Array<FixtureChannel*> fixtureChannelPoolWaiting;
    Array<Cue*> cuePoolUpdating;
    Array<Cue*> cuePoolWaiting;
    Array<Programmer*> programmerPoolUpdating;
    Array<Programmer*> programmerPoolWaiting;
    Array<Effect*> effectPoolUpdating;
    Array<Effect*> effectPoolWaiting;

    double now;

    void run() override;
    void registerFixture(Fixture* f, int id);
    void unregisterFixture(Fixture* f);
    void registerDevice(Device* f, int id);
    void unregisterDevice(Device* f);
    void registerGroup(Group* f, int id);
    void unregisterGroup(Group* f);
    void registerPreset(Preset* p, int id);
    void unregisterPreset(Preset* p);
    void registerCuelist(Cuelist* c, int id);
    void unregisterCuelist(Cuelist* c);
    void registerProgrammer(Programmer* p, int id);
    void unregisterProgrammer(Programmer* p);
    void registerCurvePreset(CurvePreset* p, int id);
    void unregisterCurvePreset(CurvePreset* p);
    void registerTimingPreset(TimingPreset* p, int id);
    void unregisterTimingPreset(TimingPreset* p);
    void registerEffect(Effect* p, int id);
    void unregisterEffect(Effect* p);

    void pleaseUpdate(Cuelist* c);
    void pleaseUpdate(FixtureChannel* f);
    void pleaseUpdate(Cue* c);
    void pleaseUpdate(Programmer* p);
    void pleaseUpdate(Effect* f);

    //void updateChannel(FixtureChannel* fc);

    Fixture* getFixtureById(int id);
    Device* getDeviceById(int id);
    Group* getGroupById(int id);
    Preset* getPresetById(int id);
    Cuelist* getCuelistById(int id);
    Programmer* getProgrammerById(int id);
    CurvePreset* getCurvePresetById(int id);
    TimingPreset* getTimingPresetById(int id);
    Effect* getEffectById(int id);

    static float symPosition(int index, int nElements);
};
