// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "InputQueue.h"
#include <Steps/StepFactory.h>
#include <Steps/Configuration.h>
#include <Steps/BuildResources.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<InputQueue> registerStep("input_queue");

    const std::string keyConsumerWaitStrategy = "consumer_wait_strategy";
    const std::string keyProducerWaitStrategy = "produder_wait_strategy";
    const std::string keyCommonWaitStrategy = "common_wait_strategy";

    const std::string keySpinCount = "spin_count";
    const std::string keyYieldCount = "yield_count";
    const std::string keySleepCount = "sleep_count";
    const std::string keySleepPeriod = "sleep_nanoseconds";
    const std::string keyMutexWaitTimeout = "timeout_nanoseconds";

    const std::string valueForever = "forever";

    const std::string keyDiscardMessagesIfNoConsumer = "discard_messages_if_no_consumer";
    const std::string keyEntryCount = "entry_count";

}


InputQueue::InputQueue()
    : connection_(new Connection)
    , discardMessagesIfNoConsumer_(false)
{
}


bool InputQueue::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyProducerWaitStrategy)
    {
        return constructWaitStrategy(configuration, parameters_.producerWaitStrategy_);
    }
    else if(key == keyConsumerWaitStrategy)
    {
        return constructWaitStrategy(configuration, parameters_.consumerWaitStrategy_);
    }
    else if(key == keyCommonWaitStrategy)
    {
        if(constructWaitStrategy(configuration, parameters_.producerWaitStrategy_))
        {
            parameters_.consumerWaitStrategy_ = parameters_.producerWaitStrategy_;
            return true;
        }
    }
    else if(key == keyDiscardMessagesIfNoConsumer)
    {
        if(configuration.getValue(discardMessagesIfNoConsumer_))
        {
            return true;
        }
        LogError("Can't interpret " << configuration.getName() << " configuration " << keyDiscardMessagesIfNoConsumer);
    }
    else if(key == keyEntryCount)
    {
        uint64_t entryCount = 0;
        if(configuration.getValue(entryCount))
        {
            parameters_.entryCount_ = size_t(entryCount);
            return true;
        }
        LogFatal("Can't interpret " << configuration.getName() << " configuration " << keyEntryCount);
        return false;
    }
    return ThreadedStepToMessage::configureParameter(key, configuration);
}


bool InputQueue::constructWaitStrategy(const ConfigurationNode & config, WaitStrategy & strategy)
{
    size_t spinCount = WaitStrategy::FOREVER;
    size_t yieldCount = WaitStrategy::FOREVER;
    size_t sleepCount = WaitStrategy::FOREVER;
    uint64_t sleepPeriod = WaitStrategy::FOREVER;
    uint64_t mutexWaitTimeout = WaitStrategy::FOREVER;

    for(auto children = config.getChildren();
        children->has();
        children->next())
    {
        auto & parameter = children->getChild();
        auto & key = parameter->getName();
        std::string valueString;
        uint64_t value = WaitStrategy::FOREVER;
        parameter->getValue(valueString);
        if(valueString != valueForever)
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
                << keySpinCount << ", "
                << keyYieldCount << ", "
                << keySleepCount << ", "
                << keySleepPeriod << ", or "
                << keyMutexWaitTimeout << ".");
            return false;
        }
    }
    LogInfo("Construct wait strategy: " << spinCount << " yield: " << yieldCount
        << " sleep: " << sleepCount << " period: " << sleepPeriod << " wait: " << mutexWaitTimeout);

    strategy = WaitStrategy(spinCount, yieldCount, sleepCount, std::chrono::nanoseconds(sleepPeriod), std::chrono::nanoseconds(mutexWaitTimeout));
    return true;
}

void InputQueue::configureResources(BuildResources & resources)
{
    resources.addConnection(name_, connection_);
    return ThreadedStepToMessage::configureResources(resources);
}

void InputQueue::attachResources(BuildResources & resources)
{
    auto pool = resources.getMemoryPool();
    connection_->createLocal(name_, parameters_, pool);
    message_.reset(new Message(pool));
    consumer_.reset(new Consumer(connection_));
}

void InputQueue::run()
{
    while(!stopping_)
    {
        if(consumer_->getNext(*message_))
        {
            auto type = message_->getType();
            send(*message_);            
        }
        else
        {
            LogTrace("InputQueue::stopped by getNext");
            stop();
        }
    }
}

void InputQueue::stop()
{
    if(consumer_)
    {
        consumer_->stop();
    }
    ThreadedStepToMessage::stop();
}

