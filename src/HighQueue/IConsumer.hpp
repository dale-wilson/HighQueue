/// @file IConsumer.h
// Copyright (c) 2016 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.hpp>

#include <HighQueue/MessageFwd.hpp>

namespace HighQueue
{
/// @brief Base for classes that consume messages from an HighQueue
class HighQueue_Export IConsumer
{
public:
    virtual ~IConsumer() {}

    /// @brief Get the next message-full of data if it is available
    ///
    /// You must call one of the Message::get() methods after a successful return
    /// from this call in order to access the data.  Do NOT save the result
    /// from a previous Message::get() call.  It will be invalidated by this call.
    ///
    /// @param message The message will be populated from the HighQueue entry.
    /// @returns immediately.  true if the message now contains data; false if no data is available.
    virtual bool tryGetNext(Message & message) = 0;

    /// @brief Get the next message-full of data.  Wait if none is available
    ///
    /// You must call one of the Message::get() methods after this call returns
    /// in order to access the data.  Do NOT save the result
    /// from a previous Message::get() call.  It will be invalidated by this call.
    ///
    /// @param message The message will be populated from the HighQueue entry.
    /// @returns true unless shutting down.
    /// Note: uses the WaitStrategy to wait.
    virtual bool getNext(Message & message) = 0;

    /// @brief for diagnosing and performance measurements, dump statistics
    virtual std::ostream & writeStats(std::ostream & out)const = 0;

    /// @brief stop receiving messages.
    virtual void stop() = 0;
    
    void setName(const std::string & name)
    {
        name_ = name;
    }

    const std::string & getName() const
    {
        return name_;
    }
private:
    std::string name_;
};
}
