// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "Builder.h"
#include <StagesSupport/ComponentBuilder.h>

#include <StagesSupport/Configuration.h>
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Connection.h>
#include <HighQueue/WaitStrategy.h>
#include <HighQueue/CreationParameters.h>
#include <StagesSupport/AsioService.h>
#include <StagesSupport/Stage.h>


using namespace HighQueue;
using namespace Stages;

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
        std::string key = child->getName();
        if(key == keyPool)
        {
            auto poolBuilder = std::make_shared<PoolBuilder>();
            if(!poolBuilder->configure(child))
            {
                return false;
            }
            pools_[poolBuilder->getName()] = poolBuilder;
        }
        else if(key == keyQueue)
        {

            auto queueBuilder = std::make_shared<QueueBuilder>(pools_);
            if(!queueBuilder->configure(child))
            {
                return false;
            }
            queues_[queueBuilder->getName()] = queueBuilder;

        }
        else if(key == keyAsio)
        {
            auto asioBuilder = std::make_shared<AsioBuilder>();
            if(!asioBuilder->configure(child))
            {
                return false;
            }
            asios_[asioBuilder->getName()] = asioBuilder;
        }
        else if(key == keyPipe)
        {
            constructPipe(child);
        }
        else
        {
            LogFatal("Unknown configuration element " << key 
                << ". Expecting: " 
                << keyPool << ", "
                << keyQueue << ", "
                << keyAsio << ", or"
                << keyPipe << ".");
            return false;
        }
    }
    return true;
}


bool Builder::constructPipe(const ConfigurationNodePtr & config)
{
    LogTrace("Pipe");
    int todo;
    return false;
}



