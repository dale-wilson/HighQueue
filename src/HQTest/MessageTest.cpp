#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Message.h>
#include <HighQueue/details/HQMemoryBLockPool.h>

using namespace HighQueue;

namespace
{
    // extra characters at beginning and end prevent the compiler from sharing this const string.
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t letterCount = 26;
}

#define DISABLE_testNormalMessagesx
#ifdef DISABLE_testNormalMessages
#pragma message ("DISABLE_testNormalMessages " __FILE__)
#else // DISABLE_testNormalMessages
BOOST_AUTO_TEST_CASE(testNormalMessages)
{
    static const size_t messageSize = sizeof(alphabet);
    static const size_t messageCount = 2;
 
    auto bytesNeeded = HQMemoryBLockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) HQMemoryBLockPool(bytesNeeded, messageSize);

    Message message1(*pool);
    BOOST_CHECK_EQUAL(message1.getUsed(), 0u);
    BOOST_CHECK(message1.isEmpty());

    message1.appendBinaryCopy(alphabet.data(), letterCount);
    BOOST_CHECK_EQUAL(message1.getUsed(), letterCount);
    BOOST_CHECK(!message1.isEmpty());

    BOOST_CHECK_EQUAL(message1.get(), message1.getConst());
    BOOST_CHECK(message1.needSpace(letterCount));
    BOOST_CHECK(!message1.needSpace(message1.available() + 1));

    std::string value1(message1.get<char>(), message1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value1);

    Message message2(*pool);
    message2.appendBinaryCopy(alphabet.data() + letterCount, letterCount);

    BOOST_CHECK_EQUAL(message2.getUsed(), letterCount);
    BOOST_CHECK(!message2.isEmpty());

    std::string value2(message2.get<char>(), message2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2);

    message1.swap(message2);

    BOOST_CHECK_EQUAL(message1.getUsed(), letterCount);
    BOOST_CHECK(!message1.isEmpty());

    std::string value1a(message1.get<char>(), message1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value1a);

    BOOST_CHECK_EQUAL(message2.getUsed(), letterCount);
    BOOST_CHECK(!message2.isEmpty());
    BOOST_CHECK_GE(message2.available(), letterCount);
    BOOST_CHECK(message2.needSpace(letterCount));

    std::string value2a(message2.get<char>(), message2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value2a);

    message1.moveTo(message2);

    BOOST_CHECK_EQUAL(message1.getUsed(), 0U);
    BOOST_CHECK(message1.isEmpty());
    BOOST_CHECK(message1.needSpace(2 * letterCount));

    BOOST_CHECK_EQUAL(message2.getUsed(), letterCount);
    BOOST_CHECK(!message2.isEmpty());

    std::string value2c(message2.get<char>(), message2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2c);

    message1.release();
    BOOST_CHECK_EQUAL(message1.getUsed(), 0U);
    BOOST_CHECK_EQUAL(message1.getOffset(), 0U);
    BOOST_CHECK(message1.isEmpty());
    BOOST_CHECK(! message1.getContainer());
    BOOST_CHECK_EQUAL(message1.available(), 0U);
    BOOST_CHECK(!message1.needSpace(1));
}
#endif // DISABLE_testNormalMessages

#define DISABLE_testMessageAppendz
#ifdef DISABLE_testMessageAppend
#pragma message ("DISABLE_testMessageAppend " __FILE__)
#else // DISABLE DISABLE_testMessageAppend
BOOST_AUTO_TEST_CASE(testMessageAppend)
{
    static const size_t messageSize = alphabet.size();
    static const size_t messageCount = 2;
    auto bytesNeeded = HQMemoryBLockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) HQMemoryBLockPool(bytesNeeded, messageSize);

    Message message(*pool);

    message.appendBinaryCopy(alphabet.data(), letterCount);
    message.appendBinaryCopy(alphabet.data() + letterCount, letterCount);

    std::string value(message.get<char>(), message.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value);
    message.setUsed(0);
    struct MonoCase
    {
        char message_[letterCount];
        MonoCase(const char * data)
        {
            std::memcpy(message_, data, sizeof(message_));
        }
    };
    MonoCase lowerCase(alphabet.data());
    MonoCase upperCase(alphabet.data() + letterCount);
    message.appendEmplace(lowerCase);
    message.appendEmplace(upperCase);
    std::string value2(message.get<char>(), message.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value2);
}
#endif // DISABLE_testMessageAppend
