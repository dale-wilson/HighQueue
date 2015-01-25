// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>

#include "Builder.h"

#include <StageCommon/Configuration.h>
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Connection.h>
#include <StageCommon/AsioService.h>
#include <StageCommon/Stage.h>

#include <Common/Log.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    const std::string queues = "queues";
    const std::string asios = "asios";
    const std::string pipes = "pipes";
    const std::string pools = "memory_pools";
}

Builder::Builder()
{

}

Builder::~Builder()
{

}

//bool build(std::string &configFileName);
//bool build(std::istream & configFile, const std::string & name);
//
//bool Builder::build(std::string &configFileName)
//{
//    BoostPropertyTreeNode config;
//    config.loadJson(configFileName);
//    return load(config);
//}
//
//bool Builder::build(std::istream & configFile, const std::string & name)
//{
//    BoostPropertyTreeNode config;
//    config.loadJson(configFile, name);
//    return load(config);
//}
            
bool Builder::construct(const ConfigurationNodePtr & config)
{
    for(auto rootChildren = config->getChildren();
        rootChildren->has();
        rootChildren->next())
    {
        auto child = rootChildren->getChild();
        std::string name = child->getName();
        if(name == pools)
        {
            constructPools(child);
        }
        else if(name == queues)
        {
            constructQueues(child);
        }
        else if(name == asios)
        {
            constructAsios(child);
        }
        else if(name == pipes)
        {
            constructPipes(child);
        }
        else
        {
            LogFatal("Unknown configuration element " << name 
                << ". Expecting: " 
                << pools << ", "
                << queues << ", "
                << asios << ", or"
                << pipes << ".");
            return false;
        }
    }
    return true;
}
bool Builder::constructPools(const ConfigurationNodePtr & config)
{
    int todo;
    return false;
}

bool Builder::constructQueues(const ConfigurationNodePtr & config)
{
    int todo;
    return false;
}

bool Builder::constructAsios(const ConfigurationNodePtr & config)
{
    int todo;
    return false;
}

bool Builder::constructPipes(const ConfigurationNodePtr & config)
{
    int todo;
    return false;
}



