// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Step_Export.h>

namespace HighQueue
{
    namespace Steps
    {
        class ConfigurationNode;
        typedef std::shared_ptr<ConfigurationNode> ConfigurationNodePtr;
        class ConfigurationChildren;
        typedef std::shared_ptr<ConfigurationChildren> ConfigurationChildrenPtr;
   }
}