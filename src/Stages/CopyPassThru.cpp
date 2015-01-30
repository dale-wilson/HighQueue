// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "CopyPassThru.h"
#include <StagesSupport/StageFactory.h>
#include <Mocks/TestMessage.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<CopyPassThru<SmallTestMessage> > registerStageSmall("small_test_message_copy");
    Registrar<CopyPassThru<MediumTestMessage> > registerStageMedium("medium_test_message_copy");
    Registrar<CopyPassThru<LargeTestMessage> > registerStageLarge("large_test_message_copy");
}
