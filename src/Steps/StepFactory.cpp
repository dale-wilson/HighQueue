// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>
#include "StepFactory.hpp"

using namespace HighQueue;
using namespace Steps;

namespace
{
    struct RegistryEntry
    {
        std::string description_;
        StepFactory::Maker maker_;
    };
    typedef std::map<std::string, RegistryEntry> Registry;
    Registry & registry()
    {
        static Registry reg;
        return reg;
    }
}

void StepFactory::registerMaker(const std::string & name, const std::string & description, const StepFactory::Maker & maker)
{
    LogTrace("StepFactory Registering " << name);
    registry()[name] = RegistryEntry{description, maker};
}

StepPtr StepFactory::make(const std::string & name)
{
    StepPtr result;
    const Registry & r = registry();
    auto pMaker = r.find(name);
    if(pMaker != r.end())
    {
        LogTrace("Step Factory making " << name);
        const Maker & maker = pMaker->second.maker_;
        result = maker();
    }
    else
    {
        if(Log::isEnabled(Log::Level::WARNING))
        {
            std::stringstream msg;
            msg << "No Step Factory Registry entry found for " << name << "." << std::endl;
            msg << "  Expecting one of: ";
            const Registry & r = registry();
            for(auto & entry : r)
            {
                msg  << std::endl << "    " << entry.first << ": " << entry.second.description_;
            }
            auto str = msg.str();
            LogWarning(str);
        }
        else
        {
            std::cout << "Log warning is disabled? " << name << std::endl;
        }
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
