// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "Builder.hpp"
#include <Steps/Configuration.hpp>
#include <Steps/Step.hpp>
#include <Steps/StepFactory.hpp>
#include <HighQueue/MemoryPool.hpp>
#include <Common/ReverseRange.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    std::string keyPipe("pipe");
    std::string keyDestination("destination");
    std::string keyComment("comment");
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
        else if(key == keyComment)
        {
            // simply ignore comments
        }
        else
        {
            LogFatal("Unknown configuration key: " << key);
            return false;
        }
    }

    // we have created all Steps, and used them to configure the build resources.
    resources_.createResources();
    for(auto & step : Steps_)
    {
        LogTrace("Attach resources for " << step->getName() << " (" << resources_.getMemoryPool()->numberOfAllocations() << ")");

        step->attachResources(resources_);
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
        if(key != keyComment)
        {
            const auto & step = StepFactory::make(key);
            if(!step)
            {
                std::cout << "Step factory denied " << key << std::endl;
                return false;
            }

#ifdef _WIN32 // VC2013 implementation of std::bind sucks (technical term)
            step->setParameterHandler(boost::bind(&Builder::configureParameter, this, _1, _2, _3));
#else // _WIN32
            step->setParameterHandler(boost::bind(&Builder::configureParameter, this, _1, _2, _3));
#endif // WIN32
            if(!step->configure(*child))
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

bool Builder::configureParameter(const StepPtr & step, const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyDestination)
    {
        return constructPipe(configuration, step);
    }
    if(key == keyComment)
    {
        return true; // simply ignore comments
    }
    return false; // false meaning "huh?"
}
