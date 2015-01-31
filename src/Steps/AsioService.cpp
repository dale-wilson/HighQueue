// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>
#include "AsioService.h"
#include <Common/Log.h>

using namespace HighQueue;
using namespace Steps;

AsioService::AsioService()
  : stopping_(false)
  , threadCount_(0)
  , threadCapacity_(0)
{
}

AsioService::~AsioService()
{
}

void
AsioService::stopService()
{
  stopping_ = true;
  ioService_.stop();
}

void
AsioService::joinThreads()
{
  while(threadCount_ > 0)
  {
     --threadCount_;
    threads_[threadCount_]->join();
    threads_[threadCount_].reset();
  }
}

void
AsioService::runThreads(size_t threadCount /*= 0*/, bool useThisThread /* = true*/)
{
  if(threadCount > threadCapacity_)
  {
    std::unique_ptr<ThreadPtr[]> newThreads(new ThreadPtr[threadCount]);
    for(size_t nThread = 0; nThread < threadCount_; ++nThread)
    {
      newThreads[nThread] = threads_[nThread];
    }
    threads_.swap(newThreads);
    threadCapacity_ = threadCount;
  }
  while(threadCount_ < threadCount)
  {
    threads_[threadCount_].reset(
      new std::thread(std::bind(&AsioService::run, this)));
    ++threadCount_;
  }
  if(useThisThread)
  {
    run();
    joinThreads();
  }
}

void
AsioService::run()
{
  long tc = 0;
  tc = ++runningThreadCount_;

  size_t count = 1;
  while(! stopping_ && count != 0)
  {
    try
    {
      count = ioService_.run();
    }
    catch (const std::exception & ex)
    {
        LogError("Error in ioservice thread.  Reset and continue: " << ex.what());
    }
  }

  --runningThreadCount_;
}

