// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "MockMessageProducer.h"
#include <Steps/StepFactory.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<MockMessageProducer<SmallMockMessage> > registerStepSmall("small_test_message_producer");
    StepFactory::Registrar<MockMessageProducer<MediumMockMessage> > registerStepMedium("medium_test_message_producer");
    StepFactory::Registrar<MockMessageProducer<LargeMockMessage> > registerStepLarge("large_test_message_producer");
}
