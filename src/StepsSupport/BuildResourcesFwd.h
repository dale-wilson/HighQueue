// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StepsSupport/Step_Export.h>

#include <StepsSupport/ComponentBuilderFwd.h>

#include <StepsSupport/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <StepsSupport/AsioServiceFwd.h>
#include <StepsSupport/StepFwd.h>
#include <HighQueue/WaitStrategyFwd.h>
#include <HighQueue/CreationParametersFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class BuildResources;
        typedef std::shared_ptr<BuildResources> BuildResourcesPtr;
   }
}