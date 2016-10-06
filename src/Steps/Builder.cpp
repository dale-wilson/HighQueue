// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "Builder.hpp"
#include <Steps/Configuration.hpp>
#include <Steps/Step.hpp>
#include <Steps/StepFactory.hpp>
#include <HighQueue/MemoryPool.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    const std::string keyPipe("pipe");
    const std::string keyDestination("destination");
    const std::string keyComment("comment");
}

Builder::Builder()
    : resources_(new SharedResources)
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
    resources_->createResources();
    return true;
}

void Builder::start()
{
    resources_->start();
}

void Builder::stop()
{
    resources_->stop();
}

void Builder::finish()
{
    resources_->finish();
}

void Builder::wait()
{
    resources_->wait();
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

            // VC2013 implementation of std::bind sucks (technical term)
            // so use boost::bind
            step->setParameterHandler(boost::bind(&Builder::configureParameter, this, _1, _2, _3));
            if(!step->configure(*child))
            {
                return false;
            }
            step->configureResources(resources_);
            resources_->addStep(step);
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
