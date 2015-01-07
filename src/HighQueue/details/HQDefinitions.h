// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/Message.h>

/// @brief HighQueue
///
/// An HighQueue is a data structure used for message passing between components of a system.  
///
/// Clients
///
/// The components that use an HighQueue will be called clients.  
///
/// Clients can be multiple processes on the same machine, multiple threads within the same process, or
/// occasionally multiple objects within a process that are serviced by the same thread.
/// 
/// There are two types of Clients, Producers, and Consumers.
///   Producers create and publish data into the HighQueue.   
///   Consumers accept and process data from the HighQueue.
///
/// Each instance of an HighQueue supports multiple Producers, but only a single Consumer.
///
/// Client API:
///
/// Clients interact with an HighQueue using an API that consists of the public interfaces of
/// four primary classes:   Message, Connection, Consumer, and Producer.
///
/// Two additonal classes are used when initializing an HighQueue: CreationParameters and ConsumerWaitStrategy.
///
/// The first step in using an HighQueue is to create an Connection using the null constructor.
///
/// This connection may be used to attach to an existing HighQueue or to create a new one using 
/// the methods: Connection::createLocal(), Connection::openOrCreateShared(), or Connection::openExistingShared(). 
/// 
/// The methods that create a new HighQueue take an instance of CreationParameters as an argument.
/// The CreationParameters in turn contains an ConsumerWaitStrategy.   See the documentation of these
/// classes for details about the parameters that can be used to configure the HighQueue for the particular use.
///
/// Once the InfinieVector is created or found the Connection can be used as the construction parameter for an instance
/// of an Producer and/or an Consumer. 
///
/// Each client should own (at least) one Message.  A Message is actually a reference to an area of memory.  As messages
/// are handled, the memory associated with any paricular Message object may change, but the 
/// client can always use its own Message object to find the message it should be working with.
///
/// The null Message constructor will create Message for use by the client.  Before a Message is first used, it must
/// be initialized with memory the memory pool within the HighQueue.   The Connection::allocate(Message &)
/// method takes care of this.
///
/// Once the Message has been populated the client may use one of the Message::get() methods to find the memory it should use.
/// Alternatively the producer can use the construct() method to construct a new object of the desired type 
/// directly in the Message's memory.
///
/// The producer may take as long as it needs to to construct a message directly in the Message.  If there are other
/// producer using the same HighQueue they will be unaffected by the time it takes a particular producer
/// to prepare its message for publication. (See the example below.)
///
/// When the producer's Message is ready it can call the Producer::publish(Message &) method.  
///
/// Important!  Once the publish method is called, the published data is no longer available to the Producer. 
/// Instead the Message will be automatically attached to a different, empty area of memory. It will be ready
/// for the producer to began creating or acquiring the next message to publish.  This means the after calling
/// publish, the producer must call Message::get() or Message::create() so it will be working with new memory area. 
///
/// The consumer receives the message by calling either of the Consumer::getNext(Message &) or 
/// Consumer::tryGetNext(Message &) methods.
///    Note: getNext() waits, tryGetNext() returns immediately a bool to indicate whether data was available.
///
/// After a getNext() or a successful tryGetNext() call, the message will point to a new area of memory.  The consumer
/// must call Message::get() again to find the newly arrived message.
///
/// A typical example: 
///
/// A publisher that is accepting and publishing incoming multicast (UDP) messages can issue the socket read request
/// using the pointer returned by Message::get() and the message size returned by Message::getCapacity().
/// When the socket read completes the publisher should use the Message::setUsed() method to record how many bytes of
/// data were received.
/// 
/// Because UDP message boundaries match transmission packet boundaries, the message is now ready to publish.
///
/// After the publish() call the Producer has an empty message that it can use for the next socket read request.
/// 
/// When the consumer's call to getNext() returns the Message::get() and Message::getUsed() methods will allow the 
/// consumer to access the memory area via its own Message object.  This is exactly the same memory area that was
/// used to read the packet from the socket. 
///
/// Users of HighQueues may stop reading here.   Developers and maintainers (and curious users) may continue
/// to:
///
/// Theory of Operation
///
/// Offsets vs Addresses
///
/// In this implementation of an HighQueue addressing is always done via offsets to a base address rather than
/// actual addresses -- up to the point where final resolution into a physical address is needed.   This has an added benefit
/// of letting different clients in different processes work effectively with an HighQueue contained in shared memory 
/// -- even if the shared memory is mapped into different virtual memory addresses in different processes.
///
/// Caveat: At the time this documentation is being written the Shared Memory suport is incomplete and there are a few issues
/// to resolve before it goes live.
///
/// Avoiding Memory Moves.
///
/// A message for the point of this discussion is a handle to a block of memory.
///
/// In spite of advances in CPU design over the years, the memcpy function still shows up regularly in profiler reports.
/// This implementation of an Infinite vector attempts to avoid memory copies by using pointers and offsets encapsulated 
/// in Messages.  A Message is a handle to a section of memory.  Two messages can be swapped with each other so that message 
/// A now points to the memory formerly held by message B, and vice versa.  This is a cheap operation compared to the 
/// cost of moving the actual data.
/// 
/// To make this work well, all messages should be the same size, and they work best if they are all allocated from the 
/// same pool of memory. In particular all the messages used in shared memory must be visible to all of the clients of
/// the HighQueue.  Thus there is a pool of memory in the HighQueue itself that can be used to populate Messages.
///
/// Cache line alignment and related performance issues.
///
/// For performance reasons many of the objects contained in the HighQueue are aligned on cache line boundaries.
/// In general the goal is to have a single writer for each cache line (although multiple readers are allowed.)  On the 
/// other hand, some high performance message passing systems pay close attention to the effect of cache-line prefetch.
/// In my tests, this effect is minor or disappears completely in a practical implementation (at least in C++), so it 
/// was ignored.  
/// 
/// Java developers of high performance systems report encountering issues related to having too many object references 
/// which must be visited by the garbage collector.   If the offset approach used in the HighQueue can be mapped 
/// into a corresponding Java implementation, this may alleviate that problem.
///
/// This cache line alignment is a tradeoff between memory usage an speed. For this implementation, speed trumps memory size.
///
/// This documentation is in HighQDefinitions.h
namespace HighQueue
{

typedef uint32_t Offset;
typedef uint32_t Signature;
typedef uint64_t Position;
typedef std::atomic<Position> AtomicPosition;
typedef std::mutex Mutex;
typedef std::condition_variable ConditionVariable;
typedef std::unique_lock<Mutex> MutexGuard;

namespace
{
    const static size_t CacheLineSize = 64;
    const static Signature InitializingSignature = 0xFEEDBABE;
    const static Signature LiveSignature = 0xFEED600D;
    const static Signature DeadSignature = 0xFEEDD1ED;
    const static uint8_t Version = 0;
} //anon


} // HighQueue
