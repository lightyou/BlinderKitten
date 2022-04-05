/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class ChannelFamily;
class ChannelType;
class FixtureTypeChannel;
class Fixture;
class Cuelist;
class Programmer;
class SubFixture;
class Effect;
class Carousel;
class Mapper;

class SubFixtureChannel{
public:
    SubFixtureChannel();
    virtual ~SubFixtureChannel();
    bool isDeleted = false;

    ChannelType* channelType;
    String objectType = "SubFixtureChannel";
    String resolution;
    float defaultValue = 0;
    float value = 0;
    float postCuelistValue = 0;
    bool isHTP = false;
    bool swopKillable = false;
    bool snapOnly = false;

    ChannelFamily* parentParamType = nullptr;
    ChannelType* parentParamDefinition = nullptr;
    FixtureTypeChannel* parentFixtureTypeChannel = nullptr;
    Fixture* parentFixture = nullptr;
    SubFixture* parentSubFixture = nullptr;

    void writeValue(float v);

    Array<Cuelist*> cuelistStack;
    Array<Programmer*> programmerStack;
    Array<Effect*> effectStack;
    Array<Carousel*> carouselStack;
    Array<Mapper*> mapperStack;
    Array<Cuelist*> cuelistFlashStack;

    void updateVal(double now);

    void cuelistOnTopOfStack(Cuelist* c);
    void cuelistOutOfStack(Cuelist* c);
    void cuelistOnTopOfFlashStack(Cuelist* c);
    void cuelistOutOfFlashStack(Cuelist* c);
    void programmerOnTopOfStack(Programmer* p);
    void programmerOutOfStack(Programmer* p);
    void effectOnTopOfStack(Effect* f);
    void effectOutOfStack(Effect* f);
    void carouselOnTopOfStack(Carousel* f);
    void carouselOutOfStack(Carousel* f);
    void mapperOnTopOfStack(Mapper* f);
    void mapperOutOfStack(Mapper* f);

};
