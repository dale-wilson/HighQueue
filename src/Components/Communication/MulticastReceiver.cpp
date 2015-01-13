// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Communication/CommunicationPch.h>
#include <Communication/MulticastReceiver.h>
#include <Communication/AsioService.h>
#include <HighQueue/Connection.h>

using namespace HighQueue;
using namespace Communication;

namespace
{
    auto ioService = std::make_shared<AsioService>();
    auto connection = std::make_shared<Connection>();
    MulticastConfiguration configuration(
        "multicastGroupIP",
        "listenInterfaceIP",
        "bindIP",
        2000);

    auto receiver1 = std::make_shared < MulticastReceiver<NullHeaderGenerator> >(
        ioService,
        connection,
        configuration);


    auto receiver2 = std::make_shared < MulticastReceiver<TypeVersionHeaderGenerator<'M','\1'> > >(
        ioService,
        connection,
        configuration);

}
