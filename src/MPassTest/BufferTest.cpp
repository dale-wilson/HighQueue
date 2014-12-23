#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <Buffers/Buffer.h>

using namespace MPass;
using namespace Buffers;

namespace
{
    // extra characters at beginning and end prevent the compiler from sharing this const string.
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string alphatwo = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t letterCount = 26;
}


BOOST_AUTO_TEST_CASE(testNormalBuffers)
{
    byte_t work1[letterCount * 2];
    std::memcpy(work1, alphabet.data(), alphabet.size());
    byte_t work2[letterCount * 2];
    std::memcpy(work2, alphabet.data(), alphabet.size());

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
    BOOST_CHECK_THROW(buffer1.appendBinaryCopy(work1, letterCount), std::runtime_error);

    buffer1.set(work1, sizeof(work1), 0, letterCount);

    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer1.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer1.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer1.getOffset(), 0U);
    BOOST_CHECK(!buffer1.isEmpty());
    BOOST_CHECK_EQUAL(work1, buffer1.getContainer());
    BOOST_CHECK_EQUAL(work1, buffer1.get());
    BOOST_CHECK_EQUAL(work1, buffer1.getConst());
    BOOST_CHECK_EQUAL(buffer1.available(), letterCount);
    BOOST_CHECK(buffer1.needSpace(letterCount));
    BOOST_CHECK(!buffer1.needSpace(letterCount + 1));
    BOOST_CHECK_EQUAL(work1 + letterCount, buffer1.getWritePosition());

    std::string value1(buffer1.get<char>(), buffer1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value1);

    Buffer buffer2;
    buffer2.set(work2, sizeof(work2), letterCount, letterCount);

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer2.getOffset(), letterCount);
    BOOST_CHECK(!buffer2.isEmpty());
    BOOST_CHECK_EQUAL(work2, buffer2.getContainer());
    BOOST_CHECK_EQUAL(&work2[letterCount], buffer2.get());
    BOOST_CHECK_EQUAL(&work2[letterCount], buffer2.getConst());
    BOOST_CHECK_EQUAL(buffer2.available(), 0u);
    BOOST_CHECK(!buffer2.needSpace(1));
    BOOST_CHECK_EQUAL(work2 + 2*letterCount, buffer2.getWritePosition());

    std::string value2(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2);

    buffer1.swap(buffer2);

    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer1.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer1.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer1.getOffset(), letterCount);
    BOOST_CHECK(!buffer1.isEmpty());
    BOOST_CHECK_EQUAL(work2, buffer1.getContainer());
    BOOST_CHECK_EQUAL(work2 + letterCount, buffer1.get());
    BOOST_CHECK_EQUAL(work2 + letterCount, buffer1.getConst());
    BOOST_CHECK_EQUAL(buffer1.available(), 0u);
    BOOST_CHECK(!buffer1.needSpace(1));
    BOOST_CHECK_EQUAL(work2 + 2 * letterCount, buffer1.getWritePosition());

    std::string value1a(buffer1.get<char>(), buffer1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value1a);

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer2.getOffset(), 0U);
    BOOST_CHECK(!buffer2.isEmpty());
    BOOST_CHECK_EQUAL(work1, buffer2.getContainer());
    BOOST_CHECK_EQUAL(work1, buffer2.get());
    BOOST_CHECK_EQUAL(work1, buffer2.getConst());
    BOOST_CHECK_EQUAL(buffer2.available(), letterCount);
    BOOST_CHECK(buffer2.needSpace(letterCount));
    BOOST_CHECK(!buffer2.needSpace(letterCount + 1));
    BOOST_CHECK_EQUAL(work1 + letterCount, buffer2.getWritePosition());

    std::string value2a(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value2a);

    buffer1.moveTo(buffer2);

    BOOST_CHECK_EQUAL(buffer1.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer1.isValid());
    BOOST_CHECK(!buffer1.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer1.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer1.getUsed(), 0U);
    BOOST_CHECK_EQUAL(buffer1.getOffset(), 0U);
    BOOST_CHECK(buffer1.isEmpty());
    BOOST_CHECK_EQUAL(work1, buffer1.getContainer());
    BOOST_CHECK_EQUAL(work1, buffer1.get());
    BOOST_CHECK_EQUAL(work1, buffer1.getConst());
    BOOST_CHECK_EQUAL(buffer1.available(), 2 * letterCount);
    BOOST_CHECK(buffer1.needSpace(2 * letterCount));
    BOOST_CHECK(!buffer1.needSpace(2 * letterCount + 1));
    BOOST_CHECK_EQUAL(work1, buffer1.getWritePosition());

    BOOST_CHECK_EQUAL(buffer2.getType(), Buffer::Type::Normal);
    BOOST_CHECK(buffer2.isValid());
    BOOST_CHECK(!buffer2.isBorrowed());
    BOOST_CHECK_NO_THROW(buffer2.mustBeNormal());
    BOOST_CHECK_EQUAL(buffer2.getUsed(), letterCount);
    BOOST_CHECK_EQUAL(buffer2.getOffset(), letterCount);
    BOOST_CHECK(!buffer2.isEmpty());
    BOOST_CHECK_EQUAL(work2, buffer2.getContainer());
    BOOST_CHECK_EQUAL(&work2[letterCount], buffer2.get());
    BOOST_CHECK_EQUAL(&work2[letterCount], buffer2.getConst());
    BOOST_CHECK_EQUAL(buffer2.available(), 0u);
    BOOST_CHECK(!buffer2.needSpace(1));
    BOOST_CHECK_EQUAL(work2 + 2*letterCount, buffer2.getWritePosition());

    std::string value2c(buffer2.get<char>(), buffer2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2c);

    buffer1.forget();
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
    BOOST_CHECK_THROW(buffer1.appendBinaryCopy(work1, letterCount), std::runtime_error);

}

BOOST_AUTO_TEST_CASE(testBorrowedBuffers)
{
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
    buffer2.set(workData, workSize, letterCount, letterCount);

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

BOOST_AUTO_TEST_CASE(testBufferAppend)
{
    byte_t work[letterCount * 2];
    std::memset(work, '\0', sizeof(work));
    Buffer buffer;
    buffer.set(work, sizeof(work), 0U);
    BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
    BOOST_CHECK_EQUAL(buffer.available(), sizeof(work));

    buffer.appendBinaryCopy(alphabet.data(), letterCount);
    buffer.appendBinaryCopy(alphabet.data() + letterCount, letterCount);
    std::string value(buffer.get<char>(), buffer.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value);
    BOOST_CHECK_THROW(buffer.appendBinaryCopy(alphabet.data(), 1), std::runtime_error);

    buffer.setUsed(0);
    struct MonoCase
    {
        char data_[letterCount];
        MonoCase(const char * data)
        {
            std::memcpy(data_, data, sizeof(data_));
        }
    };
    MonoCase lowerCase(alphabet.data());
    MonoCase upperCase(alphabet.data() + letterCount);
    buffer.appendNewCopy(lowerCase);
    buffer.appendNewCopy(upperCase);
    std::string value2(buffer.get<char>(), buffer.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value2);
    BOOST_CHECK_THROW(buffer.appendNewCopy(lowerCase), std::runtime_error);
}

/*
Still needs testing:
Split buffers
*/