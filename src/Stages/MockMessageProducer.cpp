// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "MockMessageProducer.h"
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    StageFactory::Registrar<MockMessageProducer<SmallMockMessage> > registerStageSmall("small_test_message_producer");
    StageFactory::Registrar<MockMessageProducer<MediumMockMessage> > registerStageMedium("medium_test_message_producer");
    StageFactory::Registrar<MockMessageProducer<LargeMockMessage> > registerStageLarge("large_test_message_producer");
}
