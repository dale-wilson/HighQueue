// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "Builder.h"
#include <Steps/Configuration.h>
#include <Steps/Step.h>
#include <Steps/StepFactory.h>
#include <Common/ReverseRange.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    std::string keyPipe("pipe");
    std::string keyDestination("destination");
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
        StepPtr noParent;
        auto child = rootChildren->getChild();
        const auto & key = child->getName();
        if(key == keyPipe)
        {
            if(!constructPipe(*child, noParent))
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

    // we have created all Steps, and used them to configure the build resources.
    resources_.createResources();
    for(auto & Step : Steps_)
    {
        Step->attachResources(resources_);
    }
    // now check to see if we got it right.
    for(auto & Step : Steps_)
    {
        Step->validate();
    }

    return true;
}

void Builder::start()
{
    for(auto & Step : ReverseRange<Steps>(Steps_))
    {
        Step->start();
    }
    resources_.start();
}

void Builder::stop()
{
    resources_.stop();
    for(auto & Step : Steps_)
    {
        Step->stop();
    }
}

void Builder::finish()
{
    resources_.finish();
    for(auto & Step : Steps_)
    {
        Step->finish();
    }
}

bool Builder::constructPipe(const ConfigurationNode & config, const StepPtr & parentStep)
{
    StepPtr previousStep = parentStep;
    for(auto rootChildren = config.getChildren();
        rootChildren->has();
        rootChildren->next())
    {
        auto child = rootChildren->getChild();
        const auto & key = child->getName();
        if(key == keyDestination)
        {
            constructPipe(*child, previousStep);
        }
        else
        {
            const auto & step = StepFactory::make(key);
            if(!step || !step->configure(*child))
            {
                return false;
            }
            step->configureResources(resources_);
            Steps_.emplace_back(step);
            if(previousStep)
            {
                previousStep->attachDestination(step->getName(), step);
            }
            previousStep = step;
        }
    }
    return true;
}
