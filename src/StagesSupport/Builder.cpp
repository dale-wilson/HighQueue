// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "Builder.h"
#include <StagesSupport/Configuration.h>
#include <StagesSupport/Stage.h>
#include <StagesSupport/StageFactory.h>
#include <Common/ReverseRange.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    std::string keyPipe("pipe");
}

Builder::Builder()
{
}

Builder::~Builder()
{
}
            
bool Builder::construct(const ConfigurationNode & config)
{
    for(auto rootChildren = config.getChildren();
        rootChildren->has();
        rootChildren->next())
    {
        auto child = rootChildren->getChild();
        auto & key = child->getName();
        if(key == keyPipe)
        {
            if(!constructPipe(*child))
            {
                return false;
            }
        }
        else
        {
            LogFatal("Unknown configuration key: " << key);
            return false;
        }
    }

    // we have created all stages, and used them to configure the build resources.
    resources_.createResources();
    for(auto & stage : stages_)
    {
        stage->attachResources(resources_);
    }
    // now check to see if we got it right.
    for(auto & stage : stages_)
    {
        stage->validate();
    }

    return true;
}

void Builder::start()
{

    for(auto & stage : ReverseRange<Stages>(stages_))
    {
        stage->start();
    }
    resources_.start();
}

void Builder::stop()
{
    resources_.stop();
    for(auto & stage : stages_)
    {
        stage->stop();
    }
}

void Builder::finish()
{
    resources_.finish();
    for(auto & stage : stages_)
    {
        stage->finish();
    }
}

bool Builder::constructPipe(const ConfigurationNode & config)
{
    StagePtr previousStage;

    for(auto rootChildren = config.getChildren();
        rootChildren->has();
        rootChildren->next())
    {
        auto child = rootChildren->getChild();
        auto & key = child->getName();
        auto stage = StageFactory::make(key);
        if(!stage->configure(*child))
        {
            return false;
        }
        stage->configureResources(resources_);
        stages_.emplace_back(stage);
        if(previousStage)
        {
            previousStage->attachDestination(stage);
        }
        previousStage = stage;
    }
    return true;
}
