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
const std::string ComponentBuilder::keyName = "name";
const std::string ComponentBuilder::keyMessageSize = "message_size";
const std::string ComponentBuilder::keyMessageCount = "message_count";

const std::string ComponentBuilder::keyConsumerWaitStrategy = "consumer_wait_strategy";
const std::string ComponentBuilder::keyProducerWaitStrategy = "produder_wait_strategy";
const std::string ComponentBuilder::keyCommonWaitStrategy = "common_wait_strategy";

const std::string ComponentBuilder::keySpinCount = "spin_count";
const std::string ComponentBuilder::keyYieldCount = "yield_count";
const std::string ComponentBuilder::keySleepCount = "sleep_count";
const std::string ComponentBuilder::keySleepPeriod = "sleep_nanoseconds";
const std::string ComponentBuilder::keyMutexWaitTimeout = "timeout_nanoseconds";

const std::string ComponentBuilder::valueForever = "forever";

const std::string ComponentBuilder::keyPool = "memory_pool";

const std::string ComponentBuilder::keyDiscardMessagesIfNoConsumer = "discard_messages_if_no_consumer";
const std::string ComponentBuilder::keyEntryCount = "entry_count";

const std::string ComponentBuilder::keyThreadCount = "thread_count";

ComponentBuilder::ComponentBuilder()
{}

ComponentBuilder::~ComponentBuilder()
{}

bool ComponentBuilder::configure(const ConfigurationNodePtr & config)
{
    for(auto poolChildren = config->getChildren();
        poolChildren->has();
        poolChildren->next())
    {
        auto & parameter = poolChildren->getChild();
        auto & key = parameter->getName();

        if(key == keyName)
        {
            parameter->getValue(name_);
        }
        else if(! interpretParameter(key, parameter))
        {
            return false;
        }
    }

    if(name_.empty())
    {
        LogFatal("Missing required parameter " << keyName << " for " << config->getName() << ".");
        return false;
    }
    return true;
}

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

void PoolBuilder::addToMessageCount(size_t additionalMessages)
{
    if(additionalMessages > 0)
    {
        if(messageCount_ == NONE)
        {
            messageCount_ = 0;
        }
        messageCount_ += additionalMessages;
    }
}

void PoolBuilder::needAtLeast(size_t byteCount)
{
    if(byteCount > 0)
    {     
        if(messageSize_ == NONE)
        {
            messageSize_ = 0;
        }
        if(byteCount > messageSize_)
        {
            messageSize_ = byteCount;
        }
    }
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

    for(auto children = config->getChildren();
        children->has();
        children->next())
    {
        auto & parameter = children->getChild();
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

        if(key == keySpinCount)
        {
            spinCount = size_t(value);
        }
        else if(key == keyYieldCount)
        {
            yieldCount = size_t(value);
        }
        else if(key == keySleepCount)
        {
            sleepCount = size_t(value);
        }
        else if(key == keySleepPeriod)
        {
            sleepPeriod = value;
        }
        else if(key == keyMutexWaitTimeout)
        {
            mutexWaitTimeout = value;
        }
        else
        {
            LogFatal("Unknown  wait_strategy parameter: " << key
                << ". Expecting: "
                << keyName << ", "
                << keyMessageSize << ", or"
                << keyMessageCount << ".");
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
    if(key == keyPool)
    {
        parameter->getValue(poolName_);
    }
    else if(key == keyProducerWaitStrategy)
    {
        if(!constructWaitStrategy(parameter, producerWaitStrategy_))
        {
            return false;
        }
    }
    else if(key == keyConsumerWaitStrategy)
    {
        if(!constructWaitStrategy(parameter, consumerWaitStrategy_))
        {
            return false;
        }
    }
    else if(key == keyCommonWaitStrategy)
    {
        if(!constructWaitStrategy(parameter, producerWaitStrategy_))
        {
            return false;
        }
        consumerWaitStrategy_ = producerWaitStrategy_;
    }
    else if(key == keyDiscardMessagesIfNoConsumer)
    {
        if(!parameter->getValue(discardMessagesIfNoConsumer_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << keyDiscardMessagesIfNoConsumer);
            return false;
        }
    }
    else if(key == keyEntryCount)
    {
        if(!parameter->getValue(entryCount_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << keyEntryCount);
            return false;
        }
    }
    else if(key == keyMessageSize)
    {
        if(!parameter->getValue(messageSize_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << keyMessageSize);
            return false;
        }
    }
    else if(key == keyMessageCount)
    {
        if(!parameter->getValue(messageCount_))
        {
            LogFatal("Can't interpret " << keyQueue << " parameter " << keyMessageCount);
            return false;
        }
    }
    else
    {
        LogFatal("Unknown " << keyQueue << " parameter: " << key
            << ". Expecting: "
            << keyProducerWaitStrategy << ", "
            << keyConsumerWaitStrategy << ", "
            << keyCommonWaitStrategy << ", "
            << keyDiscardMessagesIfNoConsumer << ", "
            << keyEntryCount << ", "
            << keyMessageSize << ", or "
            << keyMessageCount << ".");
        return false;
    }
    if(!poolName_.empty())
    {
        auto pPool = pools_.find(poolName_);
        if(pPool == pools_.end())
        {
            LogFatal("Unknown memory pool " << poolName_ << " while configuring queue " << name_);
            return false;
        }
        pPool->second->addToMessageCount(messageCount_);
        pPool->second->needAtLeast(messageSize_);

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
    CreationParameters parameters(producerWaitStrategy_, consumerWaitStrategy_, discardMessagesIfNoConsumer_, entryCount_, messageSize_, messageCount_);
    auto pool = pools_.at(poolName_);
    value_->createLocal(name_, parameters, pool->get());
}

PipeBuilder::PipeBuilder(
    Builder::Stages & stages, 
    Builder::Pools & pools, 
    Builder::Asios & asios, 
    Builder::Queues & queues)
    : stages_(stages)
    , pools_(pools)
    , asios_(asios)
    , queues_(queues)
{
}

PipeBuilder::~PipeBuilder()
{
}


namespace
{
    std::string stageBinaryPassThru = "binary_pass_thru";
    std::string stageCopyPassThru = "copy_pass_thru";
    std::string stageForwardPassThru = "forward_pass_thru";
    std::string stageHeartbeatProducer = "heartbeat_producer";
    std::string stageMulticastReceiver = "multicast_receiver";
    std::string stageOrderedMerge = "ordered_merge";
    std::string stageQueueConsumer = "queue_consumer";
    std::string stageQueueProducer = "queue_producer";
    std::string stageShuffle = "shuffle";
    std::string stageTee = "tee";
    std::string stageTestMessageConsumer = "test_message_consumer";
    std::string stageTestMessageProducer = "test_message_producer";
}

#include <Stages/BinaryPassThru.h>
#include <Stages/CopyPassThru.h>
#include <Stages/ForwardPassThru.h>
#include <Stages/HeartbeatProducer.h>
#include <Stages/MulticastReceiver.h>
#include <Stages/OrderedMerge.h>
#include <Stages/QueueConsumer.h>
#include <Stages/QueueProducer.h>
#include <Stages/Shuffler.h>
#include <Stages/Tee.h>
#include <Stages/TestMessageConsumer.h>
#include <Stages/TestMessageProducer.h>

bool PipeBuilder::interpretParameter(const std::string & key, ConfigurationNodePtr & parameter)
{
    
    int todo_TurnThisIntoAFactory;
    StagePtr stage;
    if(key == stageBinaryPassThru)
    {
        stage = std::make_shared<BinaryPassThru>();
    }
    //else if(key == stageCopyPassThru)
    //{
    //    stage = std::make_shared<CopyPassThru>();
    //}
    else if(key == stageForwardPassThru)
    {
        stage = std::make_shared<ForwardPassThru>();
    }
    else if(key == stageHeartbeatProducer)
    {
        stage = std::make_shared<HeartbeatProducer>();
    }
    //else if(key == stageMulticastReceiver)
    //{
    //    stage = std::make_shared<MulticastReceiver>();
    //}
    else if(key == stageOrderedMerge)
    {
        stage = std::make_shared<OrderedMerge>();
    }
    else if(key == stageQueueConsumer)
    {
        stage = std::make_shared<QueueConsumer>();
    }
    else if(key == stageQueueProducer)
    {
        stage = std::make_shared<QueueProducer>();
    }
    else if(key == stageShuffle)
    {
        stage = std::make_shared<Shuffler>();
    }
    else if(key == stageTee)
    {
        stage = std::make_shared<Tee>();
    }
    else if(key == stageTestMessageConsumer)
    {
        stage = std::make_shared<TestMessageConsumer<10>>();
    }
    else if(key == stageTestMessageProducer)
    {
        stage = std::make_shared<TestMessageProducer<10>>();
    }
    else
    {
        LogFatal("Unknown stage " << key);
        return false;
    }

    stage->configure(parameter);

    return false;
}


bool PipeBuilder::validate()
{
    int todo;
    return false;

}

void PipeBuilder::create()
{

}

