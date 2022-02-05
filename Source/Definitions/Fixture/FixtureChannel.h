/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class FixtureParamDefinition;
class DeviceTypeChannel;
class Device;
class Cuelist;

class FixtureChannel:
    public BaseItem
{
public:
    FixtureChannel(var params = var());
    virtual ~FixtureChannel();

    String objectType;
    var objectData;

    TargetParameter* channelType;
    StringParameter* resolution;
    FloatParameter* value;

    bool isHTP = false;

    FixtureParamDefinition* parentParamDefinition;
    DeviceTypeChannel* parentDeviceTypeChannel;
    Device* parentDevice;

    void writeValue(float v);
    String getTypeString() const override { return objectType; }
    static FixtureChannel* create(var params) { return new FixtureChannel(params); }

    Array<Cuelist*> cuelistStack;
    void updateVal(int64 now);

    void cuelistOnTopOfStack(Cuelist* c);
    void cuelistOutOfStack(Cuelist* c);

};
