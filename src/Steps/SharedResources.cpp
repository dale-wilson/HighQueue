// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "SharedResources.hpp"
#include <Steps/AsioService.hpp>
#include <Steps/Step.hpp>
#include <HighQueue/MemoryPool.hpp>
#include <HighQueue/details/HQMemoryBlockPool.hpp> // for diagnostic message (block count)
#include <Common/ReverseRange.hpp>

using namespace HighQueue;
using namespace Steps;

SharedResources::SharedResources()
    : numberOfMessagesNeeded_(0)
    , largestMessageSize_(0)
    , tenthsOfAsioThreadsNeeded_(0)
    , runTime_(0)
    , stopping_(false)
{
}

SharedResources::~SharedResources()
{
}

void SharedResources::addStep(const StepPtr & step)
{
    steps_.emplace_back(step);
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

   for(auto & step : steps_)
    {
        LogTrace("Attach resources for " << step->getName() << " (" << getMemoryPool()->numberOfAllocations() << ")");

        step->attachResources(shared_from_this());
    }
    // now check to see if we got it right.
    for(auto & step : steps_)
    {
        step->validate();
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
    
    timer_.reset();
    for(auto & step : ReverseRange<Steps>(steps_))
    {
        step->start();
    }
}

void SharedResources::stop()
{
    runTime_ = timer_.nanoseconds();
    if(asio_)
    {
        asio_->stopService();
    }
    for(auto & step : steps_)
    {
        step->stop();
    }
    std::unique_lock<std::mutex> lock(mutex_);
    stopping_ = true;
    condition_.notify_all();
    LogStatistics("Runtime (seconds): " << std::setprecision(9) << (double(runTime_) / double(Stopwatch::nanosecondsPerSecond)) );
}

void SharedResources::finish()
{
    if(asio_)
    {
        asio_->joinThreads();
    }
    for(auto & step : steps_)
    {
        step->finish();
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

const SharedResources::Queues & SharedResources::getQueues()const
{
    return queues_;
}

const SharedResources::Steps & SharedResources::getSteps()const
{
    return steps_;
}

void SharedResources::wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while(!stopping_)
    {
        condition_.wait(lock);
    }
}

