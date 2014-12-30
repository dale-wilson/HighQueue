// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <InfiniteVector/Buffer.h>

namespace MPass
{
/// @brief Infinite Vector
///
/// An Infinite Vector is a data structure used for message passing between components of a system.  
///
/// Clients
///
/// The components that use an Infinite Vector will be called clients.  
///
/// Clients can be multiple processes on the same machine, multiple threads within the same process, or
/// occasionally multiple objects within a process that are serviced by the same thread.
/// 
/// There are two types of Clients, Producers, and Consumers.
///   Producers create and publish data into the Infinite Vector.   
///   Consumers accept and process data from the Infinite Vector.
///
/// Each instance of an Infinite Vector supports multiple Producers, but only a single Consumer.
///
/// Client API:
///
/// Clients interact with an InfiniteVector using an API that consists of the public interfaces of
/// four primary classes:   Buffer, IvConnection, IvConsumer, and IvProducer.
///
/// Two additonal classes are used when initializing an InfiniteVector: IvCreationParameters and IvConsumerWaitStrategy.
///
/// The first step in using an InfiniteVector is to create an IvConnection using the null constructor.
///
/// This connection may be used to attach to an existing InfiniteVector or to create a new one using 
/// the methods: IvConnection::createLocal(), IvConnection::openOrCreateShared(), or IvConnection::openExistingShared(). 
/// 
/// The methods that create a new InfiniteVector take an instance of IvCreationParameters as an argument.
/// The IvCreationParameters in turn contains an IvConsumerWaitStrategy.   See the documentation of these
/// classes for details about the parameters that can be used to configure the InfiniteVector for the particular use.
///
/// Once the InfinieVector is created or found the IvConnection can be used as the construction parameter for an instance
/// of an IvProducer and/or an IvConsumer. 
///
/// Each client should own (at least) one Buffer.  A Buffer is actually a reference to an area of memory.  As messages
/// are handled, the memory associated with any paricular Buffer object may change, but the 
/// client can always use its own Buffer object to find the message it should be working with.
///
/// The null Buffer constructor will create Buffer for use by the client.  Before a Buffer is first used, it must
/// be initialized with memory the memory pool within the Infinite Vector.   The IvConnection::allocate(Buffer &)
/// method takes care of this.
///
/// Once the Buffer has been populated the client may use one of the Buffer::get() methods to find the memory it should use.
/// Alternatively the producer can use the construct() method to construct a new object of the desired type 
/// directly in the Buffer's memory.
///
/// The producer may take as long as it needs to to construct a message directly in the Buffer.  If there are other
/// producer using the same InfiniteVector they will be unaffected by the time it takes a particular producer
/// to prepare its buffer for publication. (See the example below.)
///
/// When the producer's Buffer is ready it can call the IvProducer::publish(Buffer &) method.  
///
/// Important!  Once the publish method is called, the published data is no longer available to the Producer. 
/// Instead the Buffer will be automatically attached to a different, empty area of memory. It will be ready
/// for the producer to began creating or acquiring the next message to publish.  This means the after calling
/// publish, the producer must call Buffer::get() or Buffer::create() so it will be working with new memory area. 
///
/// The consumer receives the message by calling either of the IvConsumer::getNext(Buffer &) or 
/// IvConsumer::tryGetNext(Buffer &) methods.
///    Note: getNext() waits, tryGetNext() returns immediately a bool to indicate whether data was available.
///
/// After a getNext() or a successful tryGetNext() call, the buffer will point to a new area of memory.  The consumer
/// must call Buffer::get() again to find the newly arrived message.
///
/// A typical example: 
///
/// A publisher that is accepting and publishing incoming multicast (UDP) messages can issue the socket read request
/// using the pointer returned by Buffer::get() and the buffer size returned by Buffer::getCapacity().
/// When the socket read completes the publisher should use the Buffer::setUsed() method to record how many bytes of
/// data were received.
/// 
/// Because UDP message boundaries match transmission packet boundaries, the message is now ready to publish.
///
/// After the publish() call the Producer has an empty buffer that it can use for the next socket read request.
/// 
/// When the consumer's call to getNext() returns the Buffer::get() and Buffer::getUsed() methods will allow the 
/// consumer to access the memory area via its own Buffer object.  This is exactly the same memory area that was
/// used to read the packet from the socket. 
///
/// Users of InfiniteVectors may stop reading here.   Developers and maintainers (and curious users) may continue
/// to:
///
/// Theory of Operation
///
/// InfiniteVectors vs RingBuffers
///
/// An InfiniteVector is very similar in purpose and implementation to a RingBuffer. Since a RingBuffer is a 
/// more familiar concept to most programmers, it is worth providing the reasoning behind using the 
/// InfiniteVector abstraction rather than a Ring Buffer.
///
/// Notice that neither a RingBuffer nor an InfiniteVector is a direct match for the way computer memory is
/// actually organized.  Computer memory is not organized in circles, the way a RingBuffer presents it.  Computer
/// memory is accessed linearly the same way a vector is, but it is not infinite.
///
/// As it turns out a RingBuffer an awkward abstraction to implement because the boundaries of the actual memory 
/// block used to hold the entries do not have convenient representations in ring-bufferese.  Operations such as 
/// comparing two positions within the buffer are more difficult when the buffer boundary may, or may not appear
/// between the two positions.  
/// 
/// Copying data into or out of a RingBuffer can get complex -- particularly when
/// the external source or destination is not itself contiguous. (For example when copying data directly from one
/// RingBuffer to another.)   None of these issues are show-stoppers, they just make the implementers life more 
/// difficult, and if they are not handled well, they may lead to performance issues.
///
/// It turns out that simulating infinity is easier than simulating circularity.  
///
/// For all practical purposes, the indexes into the Infinite Vector can be treated as unbounded[note below] integers.
/// They can compared incremented, etc. freely.  It is only when an index needs to be resolved into the actual 
/// address of an entry that the mapping operation need to be applied.  This operation can be localized and hidden 
/// so that most of the time it can be ignored.  (The copying data issue is handled by using referencs to fixed size
/// memory areas.)
///
/// Note: unbounded means if you use a 64 bit unsigned integer for an index and publish an entry every nanosecond, you
/// won't run out of indexes for several hundred years.
///
/// Rather than worrying about boundaries every "N" entries the way a Ring Buffer does, the Infinite Vector must be aware
/// that only the last "N" entries are visible at any time.  As this visible window moves forward in the vector, previous 
/// entries in the vector become invisible forever.  As long as the producers are careful not to get more than N entries
/// ahead of the consumer, this is not a problem.  Using unbounded indexes makes it easy for the producers to enforce this
/// limit. 
/// 
/// Offsets vs Addresses
///
/// In this implementation of an InfiniteVector addressing is always done via offsets to a base address rather than
/// actual addresses -- up to the point where final resolution into a physical address is needed.   This has an added benefit
/// of letting different clients in different processes work effectively with an InfiniteVector contained in shared memory 
/// -- even if the shared memory is mapped into different virtual memory addresses in different processes.
///
/// Caveat: At the time this documentation is being written the Shared Memory suport is incomplete and there are a few issues
/// to resolve before it goes live.
///
/// Buffers: Memory moves vs Pointer moves.
///
/// A buffer for the point of this discussion is a handle to a block of memory.
///
/// In spite of advances in CPU design over the years, the memcpy function still shows up regularly in profiler reports.
/// This implementation of an Infinite vector attempts to avoid memory copies by using pointers and offsets encapsulated 
/// in Buffers.  A Buffer is a handle to a section of memory.  Two buffers can be swapped with each other so that buffer 
/// A now points to the memory formerly held by buffer B, and vice versa.  This is a cheap operation compared to the 
/// cost of moving the actual data.
/// 
/// To make this work well, all buffers should be the same size, and they work best if they are all allocated from the 
/// same pool of memory. In particular all the buffers used in shared memory must be visible to all of the clients of
/// the InfiniteVector.  Thus there is a pool of memory in the InfiniteVector itself that can be used to populate Buffers.
///
/// Cache line alignment and related performance issues.
///
/// For performance reasons many of the objects contained in the InfiniteVector are aligned on cache line boundaries.
/// In general the goal is to have a single writer for each cache line (although multiple readers are allowed.)  On the 
/// other hand, some high performance message passing systems pay close attention to the effect of cache-line prefetch.
/// In my tests, this effect is minor or disappears completely in a practical implementation (at least in C++), so it 
/// was ignored.  
/// 
/// Java developers of high performance systems report encountering issues related to having too many object references 
/// which must be visited by the garbage collector.   If the offset approach used in the InfiniteVector can be mapped 
/// into a corresponding Java implementation, this may alleviate that problem.
///
/// This cache line alignment is a tradeoff between memory usage an speed. For this implementation, speed trumps memory size.
///
/// This documentation is in IvDefinitions.h
namespace InfiniteVector
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


} // InfiniteVector
}//MPass
