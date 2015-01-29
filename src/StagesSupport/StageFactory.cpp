// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>
#include "StageFactory.h"

using namespace HighQueue;
using namespace Stages;

namespace
{
    typedef std::map<std::string, StageFactory::Maker> Registry;
    Registry & registry()
    {
        static Registry reg;
        return reg;
    }
}

void StageFactory::registerMaker(const std::string & name, const StageFactory::Maker & maker)
{
    LogTrace("StageFactory Registering " << name);
    registry()[name] = maker;
}

StagePtr StageFactory::make(const std::string & name)
{
    StagePtr result;
    const Registry & r = registry();
    auto pMaker = r.find(name);
    if(pMaker != r.end())
    {
        LogTrace("Stage Factory making " << name);
        const Maker & maker = pMaker->second;
        result = maker();
    }
    else
    {
        LogWarning("No Stage Factory Registry entry found for " << name);
    }
    return result;
}

std::ostream & StageFactory::list(std::ostream & out)
{
    const Registry & r = registry();
    std::string delimiter = "";
    for(auto & entry : r)
    {
        out << delimiter << entry.first;
        delimiter = ", ";
    }
    return out;
}
