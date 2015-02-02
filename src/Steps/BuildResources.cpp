// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "BuildResources.h"
#include <HighQueue/MemoryPool.h>
#include <Steps/AsioService.h>

using namespace HighQueue;
using namespace Steps;

BuildResources::BuildResources()
    : numberOfMessagesNeeded_(0)
    , largestMessageSize_(0)
    , tenthsOfAsioThreadsNeeded_(0)
{
}

BuildResources::~BuildResources()
{
}
            
void BuildResources::requestAsioThread(size_t threads, size_t tenthsOfThread)
{
    tenthsOfAsioThreadsNeeded_ += threads * 10 + tenthsOfThread;
}

void BuildResources::requestMessages(size_t count)
{
    numberOfMessagesNeeded_ += count;
}
void BuildResources::requestMessageSize(size_t bytes)
{
    if(bytes > largestMessageSize_)
    {
        largestMessageSize_ = bytes;
    }
}

void BuildResources::addQueue(const std::string & name, const ConnectionPtr & connection)
{
    // TODO: check for duplicates?
    queues_[name] = connection;
}

ConnectionPtr BuildResources::findQueue(const std::string & name) const
{
    ConnectionPtr result;
    auto pConnection = queues_.find(name);
    if(pConnection != queues_.end())
    {
        result = pConnection->second;
    }
    return result;
}

void BuildResources::createResources()
{
    if(numberOfMessagesNeeded_ == 0)
    {
        throw std::runtime_error("No requests for memmory pool buffers.");
    }
    if(largestMessageSize_ == 0)
    {
        throw std::runtime_error("Memmory pool buffer size was not set.");
    }
    LogInfo("Creating Memory Pool : " << numberOfMessagesNeeded_ << " messages. " << largestMessageSize_ << " bytes each.");
    pool_ = std::make_shared<MemoryPool>(largestMessageSize_, numberOfMessagesNeeded_);

    if(tenthsOfAsioThreadsNeeded_ > 0)
    { 
        LogInfo("Creating Asio Service with " << tenthsOfAsioThreadsNeeded_ << "/10 threads.");
        asio_ = std::make_shared<AsioService>();
    }
    else
    {
        LogInfo("No requests for asio.  AsioService not created.");
    }
}

void BuildResources::start()
{
    if(asio_)
    {
        auto actualThreads = (tenthsOfAsioThreadsNeeded_ + 9) / 10;
        LogTrace("BuildResources running AsioService with " << tenthsOfAsioThreadsNeeded_ << " threads.");
        asio_->runThreads(actualThreads, false);
    }
}

void BuildResources::stop()
{
    if(asio_)
    {
        asio_->stopService();
    }
}

void BuildResources::finish()
{
    if(asio_)
    {
        asio_->joinThreads();
    }
}

const AsioServicePtr & BuildResources::getAsioService()const
{
    return asio_;
}

const MemoryPoolPtr & BuildResources::getMemoryPool()const
{
    return pool_;
}

std::string BuildResources::getQueueNames()const
{
    std::stringstream msg;
    std:: string delimiter;
    for(const auto & entry : queues_)
    {
        msg << delimiter << entry.first;
        delimiter = ", ";
    }
    return msg.str();
}
