// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "CopyPassThru.hpp"
#include <Steps/StepFactory.hpp>
#include <Mocks/MockMessage.hpp>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<CopyPassThru<SmallMockMessage> > registerStepSmall("small_test_message_copy", "Copy small message using constructor");
    StepFactory::Registrar<CopyPassThru<MediumMockMessage> > registerStepMedium("medium_test_message_copy", "Copy medium message using constructor");
    StepFactory::Registrar<CopyPassThru<LargeMockMessage> > registerStepLarge("large_test_message_copy", "Copy large message using constructor");
}
