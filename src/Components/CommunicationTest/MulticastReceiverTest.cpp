// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Communication/CommunicationPch.h"
#define BOOST_TEST_NO_MAIN MulticastReceiver
#include <boost/test/unit_test.hpp>

#include <Communication/MulticastReceiver.h>
#include <Communication/AsioService.h>
#include <HighQueue/Connection.h>

using namespace HighQueue;
using namespace Communication;

namespace
{
}

#define ENABLE_MULTICASTRECEIVERTEST 1
#if ! ENABLE_MULTICASTRECEIVERTEST
#pragma message ("ENABLE_MULTICASTRECEIVERTEST " __FILE__)
#else // ENABLE_MULTICASTRECEIVERTEST

BOOST_AUTO_TEST_CASE(MulticastReceiverTest)
{
    auto ioservice = std::make_shared<AsioService>();
    auto connection = std::make_shared<Connection>();
    MulticastConfiguration configuration("multicastGroupIP", "listenInterfaceIP", "bindIP", 10000);
    auto receiver = std::make_shared<MulticastReceiver<NullHeaderGenerator> >(ioservice, connection, configuration);
    BOOST_CHECK(false);

};

#endif // ENABLE_MULTICASTRECEIVERTEST