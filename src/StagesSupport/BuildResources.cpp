// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "BuildResources.h"
#include <HighQueue/MemoryPool.h>
#include <StagesSupport/AsioService.h>

using namespace HighQueue;
using namespace Stages;

BuildResources::BuildResources()
    : numberOfMessagesNeeded_(0)
    , largestMessageSize_(0)
    , numberOfAsioThreadsNeeded_(0)
{
}

BuildResources::~BuildResources()
{
}
            
void BuildResources::requestAsioThread(size_t count)
{
    numberOfAsioThreadsNeeded_ += count;
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

void BuildResources::createResources()
{
    if(numberOfMessagesNeeded_ == 0 || largestMessageSize_ == 0)
    {
        throw std::runtime_error("No requests for memmory pool buffers.");
    }
    pool_ = std::make_shared<MemoryPool>(largestMessageSize_, numberOfMessagesNeeded_);

    if(numberOfAsioThreadsNeeded_ > 0)
    { 
        LogInfo("Creating Asio Service with " << numberOfAsioThreadsNeeded_ << " threads.");
        asio_ = std::make_shared<AsioService>();
        asio_->runThreads(numberOfAsioThreadsNeeded_, false);
    }
    else
    {
        LogInfo("No requests for asio.  AsioService not created.");
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
