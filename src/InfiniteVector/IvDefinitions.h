#pragma once

namespace MPass
{
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
