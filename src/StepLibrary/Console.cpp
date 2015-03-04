// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "Console.hpp"
#include <Steps/StepFactory.hpp>
#include <Steps/Configuration.hpp>
#include <Steps/SharedResources.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<Console> registerStepSmall("console", "Control the system interactively.");

    const std::string keyPrompt = "prompt";
}

Console::Console()
    : prompt_("Command: ")
{
    destinationIsRequired_ = false;
}

Console::~Console()
{
}


std::ostream & Console::usage(std::ostream & out) const
{
    out << "    " << keyPrompt << ": Prompt for command." << std::endl;
    return ThreadedStepToMessage::usage(out);
}

bool Console::configureParameter(const std::string & key, const ConfigurationNode & config)
{
    if(key == keyPrompt)
    {
        return config.getValue(prompt_);
    }
    else
    {
        return ThreadedStepToMessage::configureParameter(key, config);
    }
}

void Console::attachResources(const SharedResourcesPtr & resources)
{
    resources_ = resources;
    ThreadedStepToMessage::attachResources(resources);
}


void Console::run()
{
    while(!stopping_)
    {
        std::cout << prompt_ << std::flush;
        std::string command;
        std::getline(std::cin, command);
        LogInfo("Console: " << command);

        char cmd = std::tolower(command[0]);
        switch(cmd)
        {
            case 'q':
            {
                resources_->stop();
                resources_.reset();
                break;
            }
            default:
            {
                std::cout << "Unknown command \"" << command << '"' << std::endl;
                break;
            }
        }
    }
}
