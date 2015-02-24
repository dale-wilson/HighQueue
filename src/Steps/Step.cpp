// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "Step.h"
#include <Steps/Configuration.h>
#include <Common/Log.h>

using namespace HighQueue;
using namespace Steps;

const std::string Step::keyName = "name";

Step::Step()
    : paused_(false)
    , stopping_(false)
{
}

Step::~Step()
{
    try
    {
        stop();
        finish();
    }
    catch(const std::exception & ex)
    {
        LogError("Step destruction: " << ex.what());
    }
}

bool Step::configure(const ConfigurationNode & configuration)
{
    for(auto poolChildren = configuration.getChildren();
        poolChildren->has();
        poolChildren->next())
    {
        const auto & parameter = poolChildren->getChild();
        const auto & key = parameter->getName();

        if(! configureParameter(key, *parameter))
        {
            return false;
        }
    }

    // if we don't have a name, complain now while we know what configuration we're working on.
    if(name_.empty())
    {
        LogFatal("Missing required parameter " << keyName << " for " << configuration.getName() << ".");
        return false;
    }
    return true;
}

bool Step::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyName)
    {
        configuration.getValue(name_);
        return true;
    }
    else if(parameterHandler_ && parameterHandler_(shared_from_this(), key, configuration))
    {
        return true;
    }
    std::ostringstream msg;
    msg << "Unknown parameter \"" << key << "\" for " << name_ << std::endl
        << "Expecting one of " << std::endl;
    usage(msg);
    auto str = msg.str();
    LogError(str);
    
    return false; // false meaning "huh?"
}

std::ostream & Step::usage(std::ostream & out) const
{
    out << "    " << keyName << ": Name this step (required)" << std::endl;
    return out;
}

void Step::configureResources(SharedResources & resources)
{
    // default do nothing
}


void Step::setName(const std::string & name)
{
    name_ = name;
}
const std::string & Step::getName()const
{
    return name_;
}

void Step::attachDestination(const StepPtr & destination)
{
    primaryDestination_ = destination;
}

void Step::attachDestination(const std::string & name, const StepPtr & destination)
{
    if(!primaryDestination_)
    {
        primaryDestination_ = destination;
    }
    destinations_.push_back(std::make_pair(name, destination));
}

void Step::attachResources(SharedResources & resources)
{
    // do nothing
}

void Step::validate()
{
    // do nothing
}

void Step::start()
{
}

void Step::handle(Message & message)
{
    std::stringstream msg;
    msg << "Step " << name_ << " does not accept incoming Messages.";
    throw std::runtime_error(msg.str());
}


void Step::pause()
{
    paused_ = true;
}

void Step::resume()
{
    paused_ = false;
}

void Step::stop()
{
    stopping_ = true;
}


void Step::finish()
{
    // default to do nothing.
}

void Step::mustHaveDestination()
{
    if(!primaryDestination_)
    {
        throw std::runtime_error("Configuration error: Destination required");
    }
}

void Step::mustHaveDestination(const std::string & name)
{
    if(destinationIndex(name) == ~size_t(0))
    {
        std::stringstream msg;
        msg << "Configuration error: Destination " << name << "required";
        throw std::runtime_error(msg.str());
    }
}

void Step::mustNotHaveDestination()
{
    if(primaryDestination_)
    {
        throw std::runtime_error("Configuration error: Destination not allowed");
    }
}

size_t Step::destinationIndex(const std::string & name)
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

size_t Step::getDestinationCount()const
{
    return destinations_.size();
}

void Step::setParameterHandler(ParameterHandler handler)
{
    parameterHandler_ = handler;
}


