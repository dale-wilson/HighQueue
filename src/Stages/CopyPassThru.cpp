// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "CopyPassThru.h"
#include <StagesSupport/StageFactory.h>
#include <Mocks/MockMessage.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    StageFactory::Registrar<CopyPassThru<SmallMockMessage> > registerStageSmall("small_test_message_copy");
    StageFactory::Registrar<CopyPassThru<MediumMockMessage> > registerStageMedium("medium_test_message_copy");
    StageFactory::Registrar<CopyPassThru<LargeMockMessage> > registerStageLarge("large_test_message_copy");
}
