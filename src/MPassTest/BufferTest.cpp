#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/Buffer.h>
#include <InfiniteVector/MemoryBlockPool.h>

using namespace MPass;
using namespace InfiniteVector;

namespace
{
    // extra characters at beginning and end prevent the compiler from sharing this const string.
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t letterCount = 26;
}

#define DISABLE_testNormalBuffersx
#ifdef DISABLE_testNormalBuffers
#pragma message ("DISABLE_testNormalBuffers " __FILE__)
#else // DISABLE_testNormalBuffers
BOOST_AUTO_TEST_CASE(testNormalBuffers)
{
    static const size_t bufferSize = sizeof(alphabet);
    static const size_t bufferCount = 2;
 
    auto bytesNeeded = MemoryBlockPool::spaceNeeded(bufferSize, bufferCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) MemoryBlockPool(bytesNeeded, bufferSize);

    Buffer buffer1;

    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Invalid);
    BOOST_CHECK(!buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_THROW(buffer1.mustBeNormal(), std::runtime_error);
    BOOST_CHECK_EQUAL(buffer1.getUsed(), 0U);
    BOOST_CHECK_EQUAL(buffer1.getOffset(), 0U);
    BOOST_CHECK(buffer1.isEmpty());
    BOOST_CHECK_THROW(buffer1.getContainer(), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.get(), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.getConst(), std::runtime_error);
    BOOST_CHECK_EQUAL(buffer1.available(), 0U);
    BOOST_CHECK(!buffer1.needSpace(1));
    BOOST_CHECK_THROW(buffer1.getWritePosition(), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.addUsed(1u), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.appendNewCopy(alphabet), std::runtime_error);

    pool->allocate(buffer1);
    
    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer1.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer1.getUsed(), 0u);
    BOOST_CHECK(buffer1.isEmpty());

    buffer1.appendBinaryCopy(alphabet.data(), letterCount);
    BOOST_CHECK_EQUAL(buffer1.getUsed(), letterCount);
    BOOST_CHECK(!buffer1.isEmpty());

    BOOST_CHECK_EQUAL(buffer1.get(), buffer1.getConst());
    BOOST_CHECK(buffer1.needSpace(letterCount));
    BOOST_CHECK(!buffer1.needSpace(buffer1.available() + 1));

    std::string value1(buffer1.get<char>(), buffer1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value1);

    Buffer buffer2;
    pool->allocate(buffer2);
    buffer2.appendBinaryCopy(alphabet.data() + letterCount, letterCount);

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());

    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK(!buffer2.isEmpty());

    std::string value2(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2);

    buffer1.swap(buffer2);

    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer1.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer1.getUsed(), letterCount);
    BOOST_CHECK(!buffer1.isEmpty());

    std::string value1a(buffer1.get<char>(), buffer1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value1a);

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK(!buffer2.isEmpty());
    BOOST_CHECK_GE(buffer2.available(), letterCount);
    BOOST_CHECK(buffer2.needSpace(letterCount));

    std::string value2a(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value2a);

    buffer1.moveTo(buffer2);

    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer1.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer1.getUsed(), 0U);
    BOOST_CHECK(buffer1.isEmpty());
    BOOST_CHECK(buffer1.needSpace(2 * letterCount));

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK(!buffer2.isEmpty());

    std::string value2c(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2c);

    buffer1.release();
    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Invalid);
    BOOST_CHECK(!buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_THROW(buffer1.mustBeNormal(), std::runtime_error);
    BOOST_CHECK_EQUAL(buffer1.getUsed(), 0U);
    BOOST_CHECK_EQUAL(buffer1.getOffset(), 0U);
    BOOST_CHECK(buffer1.isEmpty());
    BOOST_CHECK_THROW(buffer1.getContainer(), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.get(), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.getConst(), std::runtime_error);
    BOOST_CHECK_EQUAL(buffer1.available(), 0U);
    BOOST_CHECK(!buffer1.needSpace(1));
    BOOST_CHECK_THROW(buffer1.getWritePosition(), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.addUsed(1u), std::runtime_error);
    BOOST_CHECK_THROW(buffer1.appendNewCopy(alphabet), std::runtime_error);
}
#endif // DISABLE_testNormalBuffers

#define DISABLE_testBorrowedBuffersx
#ifdef DISABLE_testBorrowedBuffers
#pragma message ("DISABLE_testBorrowedBuffers " __FILE__)
#else // DISABLE_testBorrowedBuffers
BOOST_AUTO_TEST_CASE(testBorrowedBuffers)
{
    static const size_t bufferSize = alphabet.size();
    static const size_t bufferCount = 2;
    auto bytesNeeded = MemoryBlockPool::spaceNeeded(bufferSize, bufferCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) MemoryBlockPool(bytesNeeded, bufferSize);

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

    Buffer buffer2;
    pool->allocate(buffer2);
    buffer2.appendBinaryCopy(alphabet.data() + letterCount, letterCount);

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
    BOOST_CHECK(!buffer2.isEmpty());

    std::string value2a(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0,letterCount), value2a);
}
#endif //  DISABLE_testBorrowedBuffers

#define DISABLE_testBufferAppendz
#ifdef DISABLE_testBufferAppend
#pragma message ("DISABLE_testBufferAppend " __FILE__)
#else // DISABLE DISABLE_testBufferAppend
BOOST_AUTO_TEST_CASE(testBufferAppend)
{
    static const size_t bufferSize = alphabet.size();
    static const size_t bufferCount = 2;
    auto bytesNeeded = MemoryBlockPool::spaceNeeded(bufferSize, bufferCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) MemoryBlockPool(bytesNeeded, bufferSize);

    Buffer buffer;
    pool->allocate(buffer);

    buffer.appendBinaryCopy(alphabet.data(), letterCount);
    buffer.appendBinaryCopy(alphabet.data() + letterCount, letterCount);

    std::string value(buffer.get<char>(), buffer.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value);
    buffer.setUsed(0);
    struct MonoCase
    {
        char messageNumber_[letterCount];
        MonoCase(const char * data)
        {
            std::memcpy(messageNumber_, data, sizeof(messageNumber_));
        }
    };
    MonoCase lowerCase(alphabet.data());
    MonoCase upperCase(alphabet.data() + letterCount);
    buffer.appendNewCopy(lowerCase);
    buffer.appendNewCopy(upperCase);
    std::string value2(buffer.get<char>(), buffer.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value2);
}
#endif // DISABLE_testBufferAppend

/*
Still needs testing:
Split buffers

Memory ownership

*/