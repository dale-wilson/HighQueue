// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "MulticastSender.hpp"

#include <Steps/StepFactory.hpp>
#include <Steps/Configuration.hpp>
#include <Steps/SharedResources.hpp>

#include <Common/Log.hpp>

using namespace HighQueue;
using namespace Steps;
namespace
{
    StepFactory::Registrar<MulticastSender> registerStep("multicast_sender", "Send messages to multicast feed.");

    const std::string keyPort = "port";
    const std::string keyGroup = "group";
    const std::string keyBind = "bind";
}


MulticastSender::MulticastSender()
    : bindIP_("0.0.0.0")
    , portNumber_(0)
    , messageCount_(0)
    , errorCount_(0)
{
}

MulticastSender::~MulticastSender()
{
}

std::ostream & MulticastSender::usage(std::ostream & out) const
{
    out << "    " << keyPort << ": Port on which to send packets" << std::endl;
    out << "    " << keyGroup << ": Multicast group to which to send packets" << std::endl;
    out << "    " << keyBind << ": Identifies NIC on which to send join packets (0.0.0.0 means let the system choose)" << std::endl;
    return AsioStep::usage(out);
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
    else if(key == keyBind)
    {
        configuration.getValue(bindIP_);
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

    // Resolve the addresses to detect syntax errors here
    multicastGroup_ = Address::from_string(multicastGroupIP_);
    endpoint_ = Endpoint(multicastGroup_, portNumber_);
    bindpoint_ = Endpoint(Address::from_string(bindIP_), 0);

    AsioStep::validate();
}

void MulticastSender::start()
{
    AsioStep::start();
    socket_.reset(new Socket(*ioService_, endpoint_.protocol()));
    socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
#ifdef HIGHQUEUE_BIND_SOCKET
    // pick a NIC
    socket_->bind(bindpoint_);
#endif // HIGHQUEUE_BIND_SOCKET
}

void MulticastSender::handle(Message & message)
{
    // ignore heartbeats and shutdowns.
    auto type = message.getType();
    if(type != Message::Heartbeat && type != Message::Shutdown)
    {
        ++messageCount_;
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
}

void MulticastSender::finish()
{
    LogStatistics("Multicast Sender " << name_ << " messages: " << messageCount_);

    if(errorCount_ > 0)
    {
        LogStatistics("Multicast Sender " << name_ << " errors: " << errorCount_);
    }
}

MulticastSender::Socket  & MulticastSender::socket()
{
    return *socket_;
}

