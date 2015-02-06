// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.h>
#include <HighQueue/Connection.h>
#include <HighQueue/details/HQResolver.h>
#include <HighQueue/details/HQReservePosition.h>
#include <HighQueue/details/HQEntryAccessor.h>

namespace HighQueue
{
    /// @brief Support for publishing messages to an HighQueue
    /// In addition to having the Connection which is used to construct
    /// this object, you will need a Message which has been initialized
    /// by calling the Connection::allocate() method.
    class HighQueue_Export Producer
    {
    public:
        /// @brief Construct and attach to a connection
        /// @param connection provides access to the HighQueue
        /// @param solo indicates that this is the only producer.
        ///        solo producers run faster using techniques that would be 
        ///        unsafe with multiple producers.
        explicit Producer(ConnectionPtr & connection);  

        /// @brief Destructor
        ~Producer();
            
        /// @brief Publish the data contained in a message.
        ///
        /// If the visible window of the HighQueue is full this call
        /// waits forever.  (todo: fix this!)
        ///
        /// After this call the message will point to a different (unused)
        /// area in memory.  You must call Message::get() to find this memory
        /// area.  Do not save the result of a previous Message::get() call,
        /// this call invalidates previous get() results.
        ///
        /// @param message contains the data to be published.         
        void publish(Message & message);

        /// @brief Cancel the outstanding publish and stop publishing
        void stop();

        /// @brief for diagnosing and performance measurements, dump statistics
        std::ostream & writeStats(std::ostream & out)const;
    private:
        Position reserve();
        bool unreserve(Position position);
    private:
        ConnectionPtr connection_;
        bool solo_;
        bool stopping_;
        HQHeader * header_;
        size_t entryCount_;
        WaitStrategy waitStrategy_;
        bool consumerUsesMutex_;

        HighQResolver resolver_;
        volatile Position & readPosition_;
        volatile Position & publishPosition_;
        volatile HighQReservePosition & reserveStructure_;
        volatile AtomicPosition & reservePosition_;
        volatile Position & reserveSoloPosition_;
        SpinLock & reserveSpinLock_;
        HighQEntryAccessor entryAccessor_;

        Position publishable_;

        uint64_t statFulls_;
        uint64_t statSkips_;
        uint64_t statPublishWaits_;
        uint64_t statPublishInLine_;
        uint64_t statPublishes_;
        uint64_t statSpins_;
        uint64_t statYields_;
        uint64_t statSleeps_;
        uint64_t statWaits_;


    };
}
