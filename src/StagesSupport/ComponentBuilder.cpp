// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "ComponentBuilder.h"

#include <StagesSupport/Configuration.h>
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Connection.h>
#include <HighQueue/WaitStrategy.h>
#include <HighQueue/CreationParameters.h>
#include <StagesSupport/AsioService.h>
#include <StagesSupport/Stage.h>


using namespace HighQueue;
using namespace Stages;


////////////////////////
// ComponentBuilder
/////////////////////////
// class AsioBuilder
AsioBuilder::AsioBuilder()
    : threadCount_(0)
{}

AsioBuilder::~AsioBuilder()
{}


bool AsioBuilder::interpretParameter(const std::string & key, ConfigurationNodePtr & parameter)
{
    if(key == keyThreadCount)
    {
        if(!parameter->getValue(threadCount_))
        {
            LogFatal("Can't interpret " << keyAsio << " parameter " << keyThreadCount);
            return false;
        }
    }
    else
    {
        LogFatal("Unknown " << keyAsio << " parameter: " << key
            << ". Expecting: "
            << keyName << ", or "
            << keyThreadCount << ".");
        return false;
    }
    return true;
}

bool AsioBuilder::validate()
{
    if(threadCount_ == 0)
    {
        LogFatal("Missing required parameter " << keyThreadCount << " for " << keyAsio << ".");
        return false;
    }
    return true;
}

void AsioBuilder::create()
{
    value_ = std::make_shared<AsioService>();
    value_->runThreads(threadCount_, false);
}


//////////////////
// class PoolBuilder

PoolBuilder::PoolBuilder()
    : messageSize_(NONE)
    , messageCount_(NONE)
{
}

PoolBuilder::~PoolBuilder()
{
}

bool PoolBuilder::interpretParameter(const std::string & key, ConfigurationNodePtr & parameter)
{
    if(key == keyMessageSize && messageSize_ == NONE)
    {
        if(!parameter->getValue(messageSize_))
        {
            LogFatal("Error reading " << keyPool << " parameter " << keyMessageSize);
            return false;
        }
    }
    else if(key == keyMessageCount && messageCount_ == NONE)
    {
        if(!parameter->getValue(messageCount_))
        {
            LogFatal("Error reading " << keyPool << " parameter " << keyMessageCount);
            return false;
        }
    }
    else
    {
        LogFatal("Unknown " << keyPool << " parameter: " << key
            << ". Expecting: "
            << keyName << ", "
            << keyMessageSize << ", or"
            << keyMessageCount << ".");
        return false;
    }
    return true;
}

bool PoolBuilder::validate()
{
    if(messageCount_ == NONE)
    {
        LogFatal("Missing required parameter " << keyMessageCount << " for " << keyPool << ".");
        return false;
    }
    if(messageSize_ == NONE)
    {
        LogFatal("Missing required parameter " << keyMessageSize << " for " << keyPool << ".");
        return false;
    }
    return false;
}

void PoolBuilder::create()
{
    value_ = std::make_shared<MemoryPool>(size_t(messageSize_), size_t(messageCount_));
    LogTrace("Add memory pool " << name_ << " size=" << messageSize_ << " count=" << messageCount_);

}


////////////////////
// class QueueBuilder

QueueBuilder::QueueBuilder(Builder::Pools & pools)
    : pools_(pools)
    , entryCount_(0)
    , messageSize_(0)
    , messageCount_(0)
{}

QueueBuilder::~QueueBuilder()
{}



bool QueueBuilder::constructWaitStrategy(const ConfigurationNodePtr & config, WaitStrategy & strategy)
{
    size_t spinCount = WaitStrategy::FOREVER;
    size_t yieldCount = WaitStrategy::FOREVER;
    size_t sleepCount = WaitStrategy::FOREVER;
    uint64_t sleepPeriod = WaitStrategy::FOREVER;
    uint64_t mutexWaitTimeout = WaitStrategy::FOREVER;

    for(auto poolChildren = config->getChildren();
        poolChildren->has();
        poolChildren->next())
    {
        auto & parameter = poolChildren->getChild();
        auto & key = parameter->getName();
        std::string valueString;
        uint64_t value = WaitStrategy::FOREVER;
        parameter->getValue(valueString);
        if(valueString != PoolBuilder::valueForever)
        {
            if(!parameter->getValue(value))
            {
                LogFatal("Error reading wait strategy parameter " << key << ". Can't interpret " << valueString);
                return false;
            }
        }

        if(key == PoolBuilder::keySpinCount)
        {
            spinCount = size_t(value);
        }
        else if(key == PoolBuilder::keyYieldCount)
        {
            yieldCount = size_t(value);
        }
        else if(key == PoolBuilder::keySleepCount)
        {
            sleepCount = size_t(value);
        }
        else if(key == PoolBuilder::keySleepPeriod)
        {
            sleepPeriod = value;
        }
        else if(key == PoolBuilder::keyMutexWaitTimeout)
        {
            mutexWaitTimeout = value;
        }
        else
        {
            LogFatal("Unknown  wait_strategy parameter: " << key
                << ". Expecting: "
                << PoolBuilder::keyName << ", "
                << PoolBuilder::keyMessageSize << ", or"
                << PoolBuilder::keyMessageCount << ".");
            return false;
        }
    }
    LogTrace("Construct wait strategy: " << spinCount << " yield: " << yieldCount
        << " sleep: " << sleepCount << " period: " << sleepPeriod << " wait: " << mutexWaitTimeout);

    strategy = WaitStrategy(spinCount, yieldCount, sleepCount, std::chrono::nanoseconds(sleepPeriod), std::chrono::nanoseconds(mutexWaitTimeout));
    return true;
}


bool QueueBuilder::interpretParameter(const std::string & key, ConfigurationNodePtr & parameter)
{
    if(key == PoolBuilder::keyPool)
    {
        parameter->getValue(poolName_);
    }
    else if(key == PoolBuilder::keyProducerWaitStrategy)
    {
        if(!constructWaitStrategy(parameter, producerWaitStrategy_))
        {
            return false;
        }
    }
    else if(key == PoolBuilder::keyConsumerWaitStrategy)
    {
        if(!constructWaitStrategy(parameter, consumerWaitStrategy_))
        {
            return false;
        }
    }
    else if(key == PoolBuilder::keyCommonWaitStrategy)
    {
        if(!constructWaitStrategy(parameter, producerWaitStrategy_))
        {
            return false;
        }
        consumerWaitStrategy_ = producerWaitStrategy_;
    }
    else if(key == PoolBuilder::keyDiscardMessagesIfNoConsumer)
    {
        if(!parameter->getValue(discardMessagesIfNoConsumer_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << PoolBuilder::keyDiscardMessagesIfNoConsumer);
            return false;
        }
    }
    else if(key == PoolBuilder::keyEntryCount)
    {
        if(!parameter->getValue(entryCount_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << PoolBuilder::keyEntryCount);
            return false;
        }
    }
    else if(key == PoolBuilder::keyMessageSize)
    {
        if(!parameter->getValue(messageSize_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << PoolBuilder::keyMessageSize);
            return false;
        }
    }
    else if(key == PoolBuilder::keyMessageCount)
    {
        if(!parameter->getValue(messageCount_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << PoolBuilder::keyMessageCount);
            return false;
        }
    }
    else
    {
        LogFatal("Unknown " << keyQueue << " parameter: " << key
            << ". Expecting: "
            << PoolBuilder::keyProducerWaitStrategy << ", "
            << PoolBuilder::keyConsumerWaitStrategy << ", "
            << PoolBuilder::keyCommonWaitStrategy << ", "
            << PoolBuilder::keyDiscardMessagesIfNoConsumer << ", "
            << PoolBuilder::keyEntryCount << ", "
            << PoolBuilder::keyMessageSize << ", or "
            << PoolBuilder::keyMessageCount << ".");
        return false;
    }
    return true;
}
bool QueueBuilder::validate()
{
    // todo
    return true;
}

void QueueBuilder::create()
{
    value_ = std::make_shared<Connection>();
    LogTrace("Constructing connection: name: " << name_
        << " discard: " << discardMessagesIfNoConsumer_
        << " entries: " << entryCount_
        << " size: " << messageSize_
        << " count: " << messageCount_);

}

void QueueBuilder::start()
{
    CreationParameters parameters(producerWaitStrategy_, consumerWaitStrategy_, discardMessagesIfNoConsumer_, entryCount_, messageSize_, messageCount_);
    auto pool = pools_.at(poolName_);
    pool->create(); // todo: not now!  Shoudl be done already
    value_->createLocal(name_, parameters, pool->get());
}

