// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "TestMessageConsumer.h"
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<TestMessageConsumer<SmallTestMessage> > registerStageSmall("small_test_message_consumer");
    Registrar<TestMessageConsumer<MediumTestMessage> > registerStageMedium("medium_test_message_consumer");
    Registrar<TestMessageConsumer<LargeTestMessage> > registerStageLarge("large_test_message_consumer");
}
