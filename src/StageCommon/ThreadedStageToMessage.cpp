// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>

#include "ThreadedStageToMessage.h"

using namespace HighQueue;
using namespace Stages;
ThreadedStageToMessage::ThreadedStageToMessage()
{
}

ThreadedStageToMessage::~ThreadedStageToMessage()
{
}

void ThreadedStageToMessage::start()
{
    me_ = shared_from_this();
    thread_ = std::thread(std::bind(
        &ThreadedStageToMessage::startThread,
        this));
}


void ThreadedStageToMessage::startThread()
{
    try
    {
        run();
    }
    catch(const std::exception & ex)
    {
        LogFatal("Caught error in ThreadedStageToMessage thread: " << ex.what());
    }
    catch(...)
    {
        LogFatal("Caught unknown exceptionin ThreadedStageToMessage thread.");
        throw;
    }
}

void ThreadedStageToMessage::finish()
{
    StageToMessage::finish();
    if(me_ && std::this_thread::get_id() != thread_.get_id())
    { 
        thread_.join();
        me_.reset();
    }
}
