// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "MockMessageConsumer.h"
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    StageFactory::Registrar<MockMessageConsumer<SmallMockMessage> > registerStageSmall("small_test_message_consumer");
    StageFactory::Registrar<MockMessageConsumer<MediumMockMessage> > registerStageMedium("medium_test_message_consumer");
    StageFactory::Registrar<MockMessageConsumer<LargeMockMessage> > registerStageLarge("large_test_message_consumer");
}
