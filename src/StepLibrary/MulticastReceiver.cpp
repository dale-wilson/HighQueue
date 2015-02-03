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
    StepFactory::Registrar<MulticastReceiver> registerStep("multicast_receiver");

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
    , portNumber_(0)
{
}

MulticastReceiver::~MulticastReceiver()
{

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

    AsioStepToMessage::validate();
}

void MulticastReceiver::start()
{
    AsioStepToMessage::start();

    socket_.reset(new Socket(*ioService_));
    mcast_.reset(new MCastInfo(multicastGroupIP_, portNumber_, listenInterfaceIP_, bindIP_));
    socket_->open(mcast_->endpoint_.protocol());
    socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
    boost::asio::ip::udp::endpoint bindpoint(mcast_->bindAddress_, mcast_->portNumber_);
    socket_->bind(bindpoint);

    // Join the multicast group
    boost::asio::ip::multicast::join_group joinRequest(
        mcast_->multicastGroup_.to_v4(),
        mcast_->listenInterface_.to_v4());
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
            mcast_->senderEndpoint_,
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
    if(error)
    {
        if(canceled_)
        {
            LogError("Error in multicast reader.  Shutting down. Message: " << error);
            stop();
        }
    }
    else
    {
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
            mcast_->multicastGroup_.to_v4(),
            mcast_->listenInterface_.to_v4());
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
            mcast_->multicastGroup_.to_v4(),
            mcast_->listenInterface_.to_v4());
        socket_->set_option(joinRequest);
        joined_ = true;
    }
}

void MulticastReceiver::stop()
{
    LogTrace("HeartbeatProducer Stopping:  Cancel Timer");
    canceled_ = true;
    socket_->cancel();
    AsioStepToMessage::stop();
}

boost::asio::ip::address MulticastReceiver::listenInterface()const
{
    return mcast_->listenInterface_;
}


unsigned short MulticastReceiver::portNumber()const
{
    return mcast_->portNumber_;
}


boost::asio::ip::address MulticastReceiver::multicastGroup()const
{
    return mcast_->multicastGroup_;
}


boost::asio::ip::address MulticastReceiver::bindAddress()const
{
    return mcast_->bindAddress_;
}


boost::asio::ip::udp::endpoint MulticastReceiver::endpoint()const
{
    return mcast_->endpoint_;
}


boost::asio::ip::udp::endpoint MulticastReceiver::senderEndpoint()const
{
    return mcast_->senderEndpoint_;
}


boost::asio::ip::udp::socket & MulticastReceiver::socket()
{
    return *socket_;
}


bool MulticastReceiver::joined()const
{
    return joined_;
}
