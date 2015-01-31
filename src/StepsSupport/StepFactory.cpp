// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StepsSupport/StepPch.h>
#include "StepFactory.h"

using namespace HighQueue;
using namespace Steps;

namespace
{
    typedef std::map<std::string, StepFactory::Maker> Registry;
    Registry & registry()
    {
        static Registry reg;
        return reg;
    }
}

void StepFactory::registerMaker(const std::string & name, const StepFactory::Maker & maker)
{
    LogTrace("StepFactory Registering " << name);
    registry()[name] = maker;
}

StepPtr StepFactory::make(const std::string & name)
{
    StepPtr result;
    const Registry & r = registry();
    auto pMaker = r.find(name);
    if(pMaker != r.end())
    {
        LogTrace("Step Factory making " << name);
        const Maker & maker = pMaker->second;
        result = maker();
    }
    else
    {
        LogWarning("No Step Factory Registry entry found for " << name);
    }
    return result;
}

std::ostream & StepFactory::list(std::ostream & out)
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
