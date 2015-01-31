// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "MockMessageConsumer.h"
#include <Steps/StepFactory.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<MockMessageConsumer<SmallMockMessage> > registerStepSmall("small_test_message_consumer");
    StepFactory::Registrar<MockMessageConsumer<MediumMockMessage> > registerStepMedium("medium_test_message_consumer");
    StepFactory::Registrar<MockMessageConsumer<LargeMockMessage> > registerStepLarge("large_test_message_consumer");
}
