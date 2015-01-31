// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//
# pragma once
#include "AsioServiceFwd.h"
#include <Steps/Step_Export.h>

namespace HighQueue
{
  namespace Steps
  {
    /// @brief Base class to allow sharing a boost::io_service
    ///
    class Steps_Export AsioService
    {
    public:
      /// @brief Construct 
      AsioService();

      ~AsioService();

      /// @brief Run the event loop with this threads and threadCount additional threads.
      void runThreads(size_t threadCount = 0, bool useThisThread = true);

      /// @brief run the event loop in this thread
      ///
      /// Exceptions are caught, logged, and ignored.  The event loop continues.
      void run();

      /// @brief run the event loop until one event is handled.
      void run_one()
      {
        ioService_.run_one();
      }

      /// @brief execute any ready event handlers than return.
      size_t poll()
      {
        return ioService_.poll();
      }

      /// @brief execute at most one ready event handler than return.
      size_t poll_one()
      {
        return ioService_.poll_one();
      }

      /// @brief Allow external access
      boost::asio::io_service & ioService()
      {
        return ioService_;
      }

      /// @brief create additional threads to run the event loop
      void startThreads(size_t threadCount)
      {
        runThreads(threadCount, false);
      }

      /// @brief join all additional threads after calling stopService()
      ///
      /// If stopService() has not been called, this will block "forever".
      void joinThreads();

      /// @brief reset the IO service
      ///
      /// should be called after joinThreads before calling run*, poll*, etc. again.
      void resetService()
      {
        ioService_.reset();
        stopping_ = false;
      }

      /// @brief stop the ioservice
      void stopService();

      /// @brief allow implicit cast to io_service
      operator boost::asio::io_service &()
      {
        return ioService_;
      }

      ///@brief Dispatch to a completion handler
      /// If this is an ASIO thread, the handler might be called directly out of this method.
      /// @param handler is the handler to be posted
      template<typename CompletionHandler>
      void dispatch(CompletionHandler handler)
      {
          ioService_.dispatch(handler);
      }

      ///@brief Post a completion handler for later processing 
      /// Asio guarantees it won't be called directly out of this method.
      /// @param handler is the handler to be posted
      template<typename CompletionHandler>
      void post(CompletionHandler handler)
      {
        ioService_.post(handler);
      }

      /// @brief Attempt to determine how many threads are available to ASIO
      /// @returns the number of threads.
      uint32_t runningThreadCount()const
      {
        return runningThreadCount_;
      }

    private:
      /// Pointer to a thread
      typedef std::shared_ptr<std::thread> ThreadPtr;
      bool stopping_;
      std::unique_ptr<ThreadPtr[]> threads_;
      size_t threadCount_;
      size_t threadCapacity_;

      std::atomic<uint32_t> runningThreadCount_;
      boost::asio::io_service ioService_;
    };
  }
}
