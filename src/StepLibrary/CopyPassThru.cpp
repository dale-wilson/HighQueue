// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "CopyPassThru.h"
#include <Steps/StepFactory.h>
#include <Mocks/MockMessage.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<CopyPassThru<SmallMockMessage> > registerStepSmall("small_test_message_copy");
    StepFactory::Registrar<CopyPassThru<MediumMockMessage> > registerStepMedium("medium_test_message_copy");
    StepFactory::Registrar<CopyPassThru<LargeMockMessage> > registerStepLarge("large_test_message_copy");
}
