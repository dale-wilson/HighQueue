// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "Stage.h"
#include <StagesSupport/Configuration.h>
#include <Common/Log.h>

using namespace HighQueue;
using namespace Stages;

const std::string Stage::keyName = "name";

Stage::Stage()
    : paused_(false)
    , stopping_(false)
{
}

Stage::~Stage()
{
    try
    {
        stop();
        finish();
    }
    catch(const std::exception & ex)
    {
        LogError("Stage destruction: " << ex.what());
    }
}

bool Stage::configure(const ConfigurationNode & configuration)
{
    for(auto poolChildren = configuration.getChildren();
        poolChildren->has();
        poolChildren->next())
    {
        auto & parameter = poolChildren->getChild();
        auto & key = parameter->getName();

        if(key == keyName)
        {
            parameter->getValue(name_);
        }
        else if(! configureParameter(key, *parameter))
        {
            return false;
        }
    }

    if(name_.empty())
    {
        LogFatal("Missing required parameter " << keyName << " for " << configuration.getName() << ".");
        return false;
    }
    return true;
}

bool Stage::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    LogError("Unknown parameter \"" << key << "\" for " << name_);
    return false; // false meaning "huh?"
}

void Stage::configureResources(BuildResources & resources)
{
    // default do nothing
}


void Stage::setName(const std::string & name)
{
    name_ = name;
}
const std::string & Stage::getName()const
{
    return name_;
}

void Stage::attachDestination(const StagePtr & destination)
{
    primaryDestination_ = destination;
}

void Stage::attachDestination(const std::string & name, const StagePtr & destination)
{
    destinations_.push_back(std::make_pair(name, destination));
}

void Stage::attachResources(BuildResources & resources)
{
    // do nothing
}

void Stage::validate()
{
    // do nothing
}

void Stage::start()
{
}

void Stage::handle(Message & message)
{
    std::stringstream msg;
    msg << "Stage " << name_ << " does not accept incoming Messages.";
    throw std::runtime_error(msg.str());
}


void Stage::pause()
{
    paused_ = true;
}

void Stage::resume()
{
    paused_ = false;
}

void Stage::stop()
{
    stopping_ = true;
}


void Stage::finish()
{
    // default to do nothing.
}

void Stage::mustHaveDestination()
{
    if(!primaryDestination_)
    {
        throw std::runtime_error("Configuration error: Destination required");
    }
}

void Stage::mustHaveDestination(const std::string & name)
{
    if(destinationIndex(name) == ~size_t(0))
    {
        std::stringstream msg;
        msg << "Configuration error: Destination " << name << "required";
        throw std::runtime_error(msg.str());
    }
}

void Stage::mustNotHaveDestination()
{
    if(primaryDestination_ || destinations_.size() > 0)
    {
        throw std::runtime_error("Configuration error: Destination not allowed");
    }
}

size_t Stage::destinationIndex(const std::string & name)
{
    for(size_t index = 0; index < destinations_.size(); ++index)
    {
        auto & namedDestination = destinations_[index];
        if(namedDestination.first == name)
        {
            return index;
        }
    }
    return ~size_t(0);
}
