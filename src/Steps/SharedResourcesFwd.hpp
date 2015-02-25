// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Step_Export.hpp>

#include <Steps/ComponentBuilderFwd.hpp>

#include <Steps/ConfigurationFwd.hpp>
#include <HighQueue/MemoryPoolFwd.hpp>
#include <HighQueue/ConnectionFwd.hpp>
#include <Steps/AsioServiceFwd.hpp>
#include <Steps/StepFwd.hpp>
#include <HighQueue/WaitStrategyFwd.hpp>
#include <HighQueue/CreationParametersFwd.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class SharedResources;
        typedef std::shared_ptr<SharedResources> SharedResourcesPtr;
   }
}