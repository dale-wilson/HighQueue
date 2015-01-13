// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Communication/CommunicationPch.h>
#include <Communication/MCastReceiver.h>
#include <Communication/AsioService.h>

using namespace HighQueue;
using namespace Communication;

MCastReceiver::MCastReceiver(
    AsioServicePtr ioService,
    const std::string & multicastGroupIP,
    const std::string & listenInterfaceIP,
    const std::string & bindIP,
    unsigned short portNumber
    )
: ioService_(ioService)
, listenInterface_(boost::asio::ip::address::from_string(listenInterfaceIP))
, portNumber_(portNumber)
, multicastGroup_(boost::asio::ip::address::from_string(multicastGroupIP))
, bindAddress_(boost::asio::ip::address::from_string(bindIP))
, endpoint_(listenInterface_, portNumber)
, socket_(*ioService)
{
    // todo
}

MCastReceiver::~MCastReceiver()
{
    // todo
}

#pragma message ("HelloWorld")
