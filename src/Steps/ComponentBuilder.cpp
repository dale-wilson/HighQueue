// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#ifdef DISABLE

#include "ComponentBuilder.h"

#include <Steps/Configuration.h>
#include <Steps/StepFactory.h>
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Connection.h>
#include <HighQueue/WaitStrategy.h>
#include <HighQueue/CreationParameters.h>
#include <Steps/AsioService.h>
#include <Steps/Step.h>


using namespace HighQueue;
using namespace Steps;


////////////////////////
// ComponentBuilder



////////////////////
// class QueueBuilder

bool QueueBuilder::interpretParameter(const std::string & key, ConfigurationNodePtr & parameter)
{   if(!poolName_.empty())
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
    Builder::Steps & steps, 
    Builder::Pools & pools, 
    Builder::Asios & asios, 
    Builder::Queues & queues)
    : steps_(steps)
    , pools_(pools)
    , asios_(asios)
    , queues_(queues)
{
}

PipeBuilder::~PipeBuilder()
{
}


#include <StepLibrary/BinaryPassThru.h>
#include <StepLibrary/CopyPassThru.h>
#include <StepLibrary/ForwardPassThru.h>
#include <StepLibrary/HeartbeatProducer.h>
#include <StepLibrary/MulticastReceiver.h>
#include <StepLibrary/OrderedMerge.h>
#include <StepLibrary/InputQueue.h>
#include <StepLibrary/QueueProducer.h>
#include <StepLibrary/Shuffler.h>
#include <StepLibrary/Tee.h>
#include <StepLibrary/MockMessageConsumer.h>
#include <StepLibrary/MockMessageProducer.h>
//
//bool PipeBuilder::interpretParameter(const std::string & key, ConfigurationNodePtr & parameter, BuildResources & resources)
//{
//    StepPtr step = StepFactory::make(key);
//    if(!step)
//    {
//        LogFatal("Unknown step " << key);
//        return false;
//    }
//
//    return step->configure(parameter, resources);
//    return true;
//}
//
//
//bool PipeBuilder::validate()
//{
//    int todo;
//    return false;
//
//}
//
//void PipeBuilder::create()
//{
//
//}
//
#endif // DISABLE
