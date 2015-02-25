// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "SharedResources.hpp"
#include <Steps/AsioService.hpp>
#include <HighQueue/MemoryPool.hpp>
#include <HighQueue/details/HQMemoryBlockPool.hpp> // for diagnostic message (block count)
using namespace HighQueue;
using namespace Steps;

SharedResources::SharedResources()
    : numberOfMessagesNeeded_(0)
    , largestMessageSize_(0)
    , tenthsOfAsioThreadsNeeded_(0)
{
}

SharedResources::~SharedResources()
{
}
            
void SharedResources::requestAsioThread(size_t threads, size_t tenthsOfThread)
{
    tenthsOfAsioThreadsNeeded_ += threads * 10 + tenthsOfThread;
    LogDebug("Request Asio threads " << threads << "." << tenthsOfThread << " -> " << tenthsOfAsioThreadsNeeded_);
}

void SharedResources::requestMessages(size_t count)
{
    numberOfMessagesNeeded_ += count;
    LogDebug("Request messages: " << count << " -> " << numberOfMessagesNeeded_);
}

void SharedResources::requestMessageSize(size_t bytes)
{
    if(bytes > largestMessageSize_)
    {
        largestMessageSize_ = bytes;
    }
}

void SharedResources::addQueue(const std::string & name, const ConnectionPtr & connection)
{
    // TODO: check for duplicates?
    queues_[name] = connection;
}

ConnectionPtr SharedResources::findQueue(const std::string & name) const
{
    ConnectionPtr result;
    auto pConnection = queues_.find(name);
    if(pConnection != queues_.end())
    {
        result = pConnection->second;
    }
    return result;
}

void SharedResources::createResources()
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
    auto & detail = pool_->getPool();
    LogDebug("Memory pool contains " << detail.getBlockCount() << " blocks of " << detail.getBlockCapacity() << " bytes.");

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

void SharedResources::start()
{
    if(asio_)
    {
        auto actualThreads = (tenthsOfAsioThreadsNeeded_ + 9) / 10;
        LogTrace("SharedResources running AsioService with " << actualThreads << " threads.");
        asio_->runThreads(actualThreads, false);
    }
}

void SharedResources::stop()
{
    if(asio_)
    {
        asio_->stopService();
    }
}

void SharedResources::finish()
{
    if(asio_)
    {
        asio_->joinThreads();
    }
}

const AsioServicePtr & SharedResources::getAsioService()const
{
    return asio_;
}

const MemoryPoolPtr & SharedResources::getMemoryPool()const
{
    return pool_;
}

std::string SharedResources::getQueueNames()const
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
