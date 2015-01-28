// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>

namespace HighQueue
{
    namespace Stages
    {
        class ConfigurationNode;
        typedef std::shared_ptr<ConfigurationNode> ConfigurationNodePtr;
        class ConfigurationChildren;
        typedef std::shared_ptr<ConfigurationChildren> ConfigurationChildrenPtr;
   }
}