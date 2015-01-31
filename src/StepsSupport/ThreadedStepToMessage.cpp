// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StepsSupport/StepPch.h>

#include "ThreadedStepToMessage.h"

using namespace HighQueue;
using namespace Steps;
ThreadedStepToMessage::ThreadedStepToMessage()
{
}

ThreadedStepToMessage::~ThreadedStepToMessage()
{
}

void ThreadedStepToMessage::start()
{
    me_ = shared_from_this();
    thread_ = std::thread(std::bind(
        &ThreadedStepToMessage::startThread,
        this));
}


void ThreadedStepToMessage::startThread()
{
    try
    {
        run();
    }
    catch(const std::exception & ex)
    {
        LogFatal("Caught error in ThreadedStepToMessage thread: " << ex.what());
    }
    catch(...)
    {
        LogFatal("Caught unknown exceptionin ThreadedStepToMessage thread.");
        throw;
    }
}

void ThreadedStepToMessage::finish()
{
    StepToMessage::finish();
    if(me_ && std::this_thread::get_id() != thread_.get_id())
    { 
        thread_.join();
        me_.reset();
    }
}
