/// @file FanInConsumer.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/Consumer.hpp>
#include <HighQueue/ConnectionFwd.hpp>

namespace HighQueue
{
    /// @brief Support for consuming messages from an multiple HighQueues
    class HighQueue_Export MultiQueueConsumer : public IConsumer
    {
    public:
        /// @brief Construct
        MultiQueueConsumer();

        virtual ~MultiQueueConsumer();

        /// @brief Add a new queue to the collection we are listening to.
        /// @param connection Is the connection to the HighQueue
        void addQueue(ConnectionPtr & connection);

        /// @brief Get the next message-full of data if it is available
        ///
        /// You must call one of the Message::get() methods after a successful return
        /// from this call in order to access the data.  Do NOT save the result
        /// from a previous Message::get() call.  It will be invalidated by this call.
        ///
        /// @param message The message will be populated from the HighQueue entry.
        /// @returns immediately.  true if the message now contains data; false if no data is available.
        virtual bool tryGetNext(Message & message);

        /// @brief Get the next message-full of data.  Wait if none is available
        ///
        /// You must call one of the Message::get() methods after this call returns
        /// in order to access the data.  Do NOT save the result
        /// from a previous Message::get() call.  It will be invalidated by this call.
        ///
        /// @param message The message will be populated from the HighQueue entry.
        /// @returns true unless shutting down.
        /// Note: uses the WaitStrategy to wait.
        virtual bool getNext(Message & message);

        /// @brief for diagnosing and performance measurements, dump statistics
        virtual std::ostream & writeStats(std::ostream & out)const;

        /// @brief stop receiving messages.
        virtual void stop();

    private:
        size_t nextConsumer();

    private:
        typedef std::shared_ptr<IConsumer> ConsumerPtr;
        typedef std::vector<ConsumerPtr> ConsumerVec;
        ConsumerVec consumers_;
        size_t consumerPos_;

        bool stopping_;
    };
}
