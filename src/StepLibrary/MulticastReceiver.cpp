// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "MulticastReceiver.h"

#include <Steps/StepFactory.h>
#include <Steps/Configuration.h>
#include <Steps/SharedResources.h>

#include <Common/Log.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<MulticastReceiver> registerStep("multicast_receiver", "Receive messages from multicast feed.");

    const std::string keyPacketSize = "packet_size";
    const std::string keyPort = "port";
    const std::string keyGroup = "group";
    const std::string keyListen = "listen";
    const std::string keyBind = "bind";
}


MulticastReceiver::MulticastReceiver()
    : joined_(false)
    , canceled_(false)
    , packetSize_(0)
    , listenInterfaceIP_("0.0.0.0")
    , bindIP_("0.0.0.0")
    , portNumber_(0)
    , messagesReceived_(0)
{
}

MulticastReceiver::~MulticastReceiver()
{

}

std::ostream & MulticastReceiver::usage(std::ostream & out) const
{
    out << "    " << keyPacketSize << ": Expected maximum message size and/or UDP MTU" << std::endl;
    out << "    " << keyPort << ": Port on which to listen" << std::endl;
    out << "    " << keyGroup << ": Multicast group to join" << std::endl;
    out << "    " << keyListen << ": Identifies NIC on which to listen (0.0.0.0 lets the system choose)" << std::endl;
    out << "    " << keyBind << ": Identifies NIC on which to send join request (almost always = listen)" << std::endl;
    return AsioStepToMessage::usage(out);
}

bool MulticastReceiver::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyPacketSize)
    {
        uint64_t packetSize;
        if(!configuration.getValue(packetSize))
        {
            LogError("Can't interpret parameter \"" << key << "\" for " << name_);
            return false;
        }
        packetSize_ = size_t(packetSize);
    }
    else if(key == keyPort)
    {
        uint64_t port;
        if(!configuration.getValue(port))
        {
            LogError("Can't interpret parameter \"" << key << "\" for " << name_);
            return false;
        }
        portNumber_ = uint16_t(port);
    }
    else if(key == keyGroup)
    {
        configuration.getValue(multicastGroupIP_);
    }
    else if(key == keyListen)
    {
        configuration.getValue(listenInterfaceIP_);
    }
    else if(key == keyBind)
    {
        configuration.getValue(bindIP_);
    }
    else
    {
        return AsioStepToMessage::configureParameter(key, configuration);
    }
    return true;
}

void MulticastReceiver::configureResources(SharedResources & resources)
{
    resources.requestMessageSize(packetSize_);
    AsioStepToMessage::configureResources(resources);
}

void MulticastReceiver::validate()
{
    if(packetSize_ == 0)
    {
        std::stringstream msg;
        msg << "Multicast Receiver missing configuration parameter " << keyPacketSize;
        throw std::runtime_error(msg.str());
    }

    LogDebug("Multicast Receiver Group IP: " << multicastGroupIP_);
    multicastGroup_ = Address::from_string(multicastGroupIP_);
    LogInfo("Multicast Receiver Listen IP: " << listenInterfaceIP_);
    listenInterface_ = Address::from_string(listenInterfaceIP_);
    listenEndpoint_ = Endpoint(listenInterface_, portNumber_);
    LogDebug("Multicast Receiver Bind IP: " << bindIP_);
    bindAddress_ = Address::from_string(bindIP_);
    bindpoint_ = Endpoint(bindAddress_, portNumber_);

    AsioStepToMessage::validate();
}

void MulticastReceiver::start()
{
    AsioStepToMessage::start();


    LogDebug("Multicast receiver open socket: " << listenEndpoint_);
    socket_.reset(new Socket(*ioService_));
    socket_->open(listenEndpoint_.protocol());
    LogDebug("Multicast receiver set reuse");
    socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_->bind(bindpoint_);

    // Join the multicast group
    LogDebug("Multicast Receiver " << multicastGroup_.to_v4() << " listen: " << listenInterface_.to_v4());
    boost::asio::ip::multicast::join_group joinRequest(
        multicastGroup_.to_v4(),
        listenInterface_.to_v4());
    socket_->set_option(joinRequest);
    joined_ = true;

    startRead();
}


void MulticastReceiver::startRead()
{
    if(!stopping_)
    {
        socket_->async_receive_from(
            boost::asio::buffer(outMessage_->getWritePosition(), outMessage_->available()),
            senderEndpoint_,
            boost::bind(&MulticastReceiver::handleReceive,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
            );
    }
}


void MulticastReceiver::handleReceive(
    const boost::system::error_code& error,
    size_t bytesReceived)
{
    LogDebug("MulticastReceiver handleReceive");
    if(error)
    {
        if(!canceled_)
        {
            LogError("Error in multicast reader: " << error.message());
        }
    }
    else
    {
        ++messagesReceived_;
        outMessage_->setType(Message::MulticastPacket);
        auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        outMessage_->setTimestamp(timestamp);
        outMessage_->addUsed(bytesReceived);
        send(*outMessage_);
        startRead();
    }
}


void MulticastReceiver::pause()
{
    // Temporarily leave the group
    if(joined_)
    {
        boost::asio::ip::multicast::leave_group leaveRequest(
            multicastGroup_.to_v4(),
            listenInterface_.to_v4());
        socket_->set_option(leaveRequest);
        joined_ = false;
    }
    AsioStepToMessage::pause();
}


void MulticastReceiver::resume()
{
    AsioStepToMessage::resume();
    if(!joined_)
    {
        // rejoin the multicast group
        boost::asio::ip::multicast::join_group joinRequest(
            multicastGroup_.to_v4(),
            listenInterface_.to_v4());
        socket_->set_option(joinRequest);
        joined_ = true;
    }
}

void MulticastReceiver::stop()
{
    LogTrace("Multicast Receiver Stopping:  Cancel Read");
    canceled_ = true;
    socket_->cancel();
    AsioStepToMessage::stop();
}

MulticastReceiver::Endpoint MulticastReceiver::senderEndpoint()const
{
    return senderEndpoint_;
}

bool MulticastReceiver::joined()const
{
    return joined_;
}
