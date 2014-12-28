#include "Common/MPassPch.h"

#define BOOST_TEST_NO_MAIN MPassPerformanceTest
#include <boost/test/unit_test.hpp>

#include <Buffers/Buffer.h>
#include <Common/Stopwatch.h>

using namespace MPass;
using namespace Buffers;

namespace
{
    // extra characters at beginning and end prevent the compiler from sharing this const string.
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string alphatwo = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t letterCount = 26;
}

#define DISABLE_BUFFER_MOVESWAP_PERFORMANCE
#ifndef DISABLE_BUFFER_MOVESWAP_PERFORMANCE
BOOST_AUTO_TEST_CASE(testBufferMoveSwapPerformance)
{
    byte_t work1[letterCount * 2];
    std::memcpy(work1, alphabet.data(), alphabet.size());
    byte_t work2[letterCount * 2];
    std::memcpy(work2, alphabet.data(), alphabet.size());
    Buffer::MemoryOwnerPtr owner;

    Buffer buffer1;
    buffer1.set(owner, work1, sizeof(work1), 0, letterCount);
    Buffer buffer2;
    buffer2.set(owner, work2, sizeof(work2), letterCount, letterCount);

    size_t limit1 = 10;//100000;
    size_t limit2 = 1;//10000;
    size_t totalOps = 2*limit1*limit2;
    double nanoseconds = 1E9;

    Stopwatch timer;
    timer.reset();
    for(size_t i = 0; i < 100000; ++i)for(size_t j = 0; j < 10000; ++j)
    {
        buffer1.moveTo(buffer2);
        buffer2.moveTo(buffer1);
    }
    auto moveNormalLapse = timer.microseconds();
    timer.reset();
    for(size_t i = 0; i < 100000; ++i)for(size_t j = 0; j < 10000; ++j)
    {
        buffer1.swap(buffer2);
        buffer2.swap(buffer1);
    }
    auto swapLapse = timer.microseconds();

    std::cout << "Swap: " << swapLapse << " " << (double(swapLapse) / double(totalOps)) * 1000.0L
        << "  Move Normal: " << moveNormalLapse << " " << (double(moveNormalLapse) / double(totalOps)) * 1000.0L
        << std::endl;


    BOOST_CHECK(true);

}
#endif // DISABLE_BUFFER_MOVESWAP_PERFORMANCE

#define DISABLE_BORROWED_BUFFER_PERFORMANCE
#ifndef DISABLE_BORROWED_BUFFER_PERFORMANCE
BOOST_AUTO_TEST_CASE(testBorrowedBuffers)
{
    Buffer::MemoryOwnerPtr owner;

    Buffer buffer1;
    auto data = reinterpret_cast<const byte_t *>(alphabet.data());
    buffer1.borrow(data, 0u, letterCount);
    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Borrowed);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(buffer1.isBorrowed());
    BOOST_CHECK_THROW(buffer1.mustBeNormal(), std::runtime_error);
    BOOST_CHECK_EQUAL(buffer1.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer1.getOffset(), 0U);
    BOOST_CHECK(!buffer1.isEmpty());
    BOOST_CHECK_EQUAL(data, buffer1.getContainer());
    BOOST_CHECK_THROW(buffer1.get(), std::runtime_error);
    BOOST_CHECK_EQUAL(data, buffer1.getConst());
    BOOST_CHECK_EQUAL(buffer1.available(), 0);
    BOOST_CHECK(!buffer1.needSpace(1));
    BOOST_CHECK_THROW(buffer1.getWritePosition(), std::runtime_error);

    std::string workString(alphabet);
    auto workData = const_cast<byte_t *>(reinterpret_cast<const byte_t *>(workString.c_str()));
    auto workSize = workString.size();

    Buffer buffer2;
    buffer2.set(owner, workData, workSize, letterCount, letterCount);

    std::string value2(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2);

    BOOST_CHECK_THROW(buffer1.swap(buffer2), std::runtime_error);
    BOOST_CHECK_THROW(buffer2.swap(buffer1), std::runtime_error);
    BOOST_CHECK_THROW(buffer2.moveTo(buffer1), std::runtime_error);
    BOOST_CHECK_NO_THROW(buffer1.moveTo(buffer2));

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer2.getOffset(), letterCount);
    BOOST_CHECK(!buffer2.isEmpty());
    BOOST_CHECK_EQUAL(workData, buffer2.getContainer());
    BOOST_CHECK_EQUAL(workData + letterCount, buffer2.get());
    BOOST_CHECK_EQUAL(workData + letterCount, buffer2.getConst());
    BOOST_CHECK_EQUAL(buffer2.available(), 0);
    BOOST_CHECK(!buffer2.needSpace(1));
    BOOST_CHECK_EQUAL(workData + 2 * letterCount, buffer2.getWritePosition());

    std::string value2a(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0,letterCount), value2a);

    BOOST_CHECK_NE(alphabet, workString);
}
#endif // DISABLE_BORROWED_BUFFER_PERFORMANCE
