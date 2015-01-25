// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "Builder.h"

#include <StagesSupport/Configuration.h>
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Connection.h>
#include <StagesSupport/AsioService.h>
#include <StagesSupport/Stage.h>

#include <Common/Log.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    // Top level
    const std::string nameQueue = "queue";
    const std::string nameAsio = "asio";
    const std::string namePipe = "pipe";
    const std::string namePool = "memory_pool";

    const std::string nameName = "name";
    const std::string nameMessageSize = "message_size";
    const std::string nameMessageCount = "message_count";

}

Builder::Builder()
{
}

Builder::~Builder()
{
}
            
bool Builder::construct(const ConfigurationNodePtr & config)
{
    for(auto rootChildren = config->getChildren();
        rootChildren->has();
        rootChildren->next())
    {
        auto child = rootChildren->getChild();
        std::string name = child->getName();
        if(name == namePool)
        {
            constructPool(child);
        }
        else if(name == nameQueue)
        {
            constructQueue(child);
        }
        else if(name == nameAsio)
        {
            constructAsio(child);
        }
        else if(name == namePipe)
        {
            constructPipe(child);
        }
        else
        {
            LogFatal("Unknown configuration element " << name 
                << ". Expecting: " 
                << namePool << ", "
                << nameQueue << ", "
                << nameAsio << ", or"
                << namePipe << ".");
            return false;
        }
    }
    return true;
}

bool Builder::constructPool(const ConfigurationNodePtr & config)
{
    static const size_t NONE = ~size_t(0);

    std::string name;
    uint64_t messageSize = NONE;
    uint64_t messageCount = NONE;

    for(auto poolChildren = config->getChildren();
        poolChildren->has();
        poolChildren->next())
    {
        auto & parameter = poolChildren->getChild();
        auto & key = parameter->getName();
        if(key == nameName && name.empty())
        {
            if(!parameter->getValue(name))
            {
                LogFatal("Error reading " << namePool << " parameter " << nameName);
                return false;
            }
        }
        else if(key == nameMessageSize && messageSize == NONE)
        {
            if(!parameter->getValue(messageSize))
            {
                LogFatal("Error reading " << namePool << " parameter " << nameMessageSize);
                return false;
            }
        }
        else if(key == nameMessageCount && messageCount == NONE)
        {
            if(!parameter->getValue(messageCount))
            {
                LogFatal("Error reading " << namePool << " parameter " << nameMessageCount);
                return false;
            }
        }
        else
        {
            LogFatal("Unknown " << namePool << " parameter: " << key
                << ". Expecting: "
                << nameName << ", "
                << nameMessageSize << ", or"
                << nameMessageCount << ".");
            return false;
        }
    }
    if(name.empty())
    {
        LogFatal("Missing required parameter " << nameName << " for " << namePool << ".");
        return false;
    }
    if(messageCount == NONE)
    {
        LogFatal("Missing required parameter " << nameMessageCount << " for " << namePool << ".");
        return false;
    }
    if(messageSize == NONE)
    {
        LogFatal("Missing required parameter " << nameMessageSize << " for " << namePool << ".");
        return false;
    }
    auto pool = std::make_shared<MemoryPool>(size_t(messageSize), size_t(messageCount));
    LogTrace("Add memory pool " << name << " size=" << messageSize << " count=" << messageCount);
    pools_[name] = pool;

    return true;
}

bool Builder::constructQueue(const ConfigurationNodePtr & config)
{
    LogTrace("Queue");
    int todo;
    return false;
}

bool Builder::constructAsio(const ConfigurationNodePtr & config)
{
    LogTrace("Asio");
    int todo;
    return false;
}

bool Builder::constructPipe(const ConfigurationNodePtr & config)
{
    LogTrace("Pipe");
    int todo;
    return false;
}



