// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "MulticastSender.h"

#include <Steps/StepFactory.h>
#include <Steps/Configuration.h>
#include <Steps/SharedResources.h>

#include <Common/Log.h>

using namespace HighQueue;
using namespace Steps;
namespace
{
    StepFactory::Registrar<MulticastSender> registerStep("multicast_sender");

    const std::string keyPort = "port";
    const std::string keyGroup = "group";
}


MulticastSender::MulticastSender()
    : canceled_(false)
    , portNumber_(0)
    , errorCount_(0)
{
}

MulticastSender::~MulticastSender()
{
}

bool MulticastSender::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyPort)
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
    else
    {
        return AsioStep::configureParameter(key, configuration);
    }
    return true;
}

void MulticastSender::configureResources(SharedResources & resources)
{
    AsioStep::configureResources(resources);
}

void MulticastSender::validate()
{
    if(portNumber_ == 0)
    {
        std::stringstream msg;
        msg << "Multicast Receiver missing configuration parameter " << keyPort;
        throw std::runtime_error(msg.str());
    }
    if(multicastGroupIP_.empty())
    {
        std::stringstream msg;
        msg << "Multicast Receiver missing configuration parameter " << keyGroup;
        throw std::runtime_error(msg.str());
    }

    AsioStep::validate();
}

void MulticastSender::start()
{
    AsioStep::start();
    multicastGroup_ = Address::from_string(multicastGroupIP_);

    endpoint_ = boost::asio::ip::udp::endpoint(multicastGroup_, portNumber_);
    socket_.reset(new Socket(*ioService_, endpoint_.protocol()));
    socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
    if(!bindIP_.empty())
    {
        // pick a NIC
        Endpoint bindpoint(Address::from_string(bindIP_), portNumber_);
        socket_->bind(bindpoint);
    }
}

void MulticastSender::handle(Message & message)
{
    boost::system::error_code error;
    Socket::message_flags flags(0);
    socket_->send_to(
        boost::asio::buffer(message.getConst(), message.getUsed()),
        endpoint_,
        flags,
        error);
    if(error)
    {
        LogError("Multicast Sender " << name_ << " error: " << error.message());
        ++errorCount_;
    }
}

void MulticastSender::stop()
{
    LogTrace("HeartbeatProducer Stopping:  Cancel Timer");
    canceled_ = true;
    socket_->cancel();
    AsioStep::stop();
}

void MulticastSender::finish()
{
    if(errorCount_ > 0)
    {
        LogStatistics("Multicast Sender " << name_ << " errors: " << errorCount_);
    }
}

MulticastSender::Socket  & MulticastSender::socket()
{
    return *socket_;
}

