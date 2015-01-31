// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Step_Export.h>

#include <Steps/ComponentBuilderFwd.h>

#include <Steps/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <Steps/AsioServiceFwd.h>
#include <Steps/StepFwd.h>
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