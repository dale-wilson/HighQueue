// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>

#include <StagesSupport/ComponentBuilderFwd.h>

#include <StagesSupport/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <StagesSupport/AsioServiceFwd.h>
#include <StagesSupport/StageFwd.h>
#include <HighQueue/WaitStrategyFwd.h>
#include <HighQueue/CreationParametersFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class BuildResources;
        typedef std::shared_ptr<BuildResources> BuildResourcesPtr;
   }
}