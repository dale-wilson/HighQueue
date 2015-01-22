#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Message.h>
#include <HighQueue/details/HQMemoryBlockPool.h>

using namespace HighQueue;

namespace
{
    // extra characters at beginning and end prevent the compiler from sharing this const string.
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t letterCount = 26;
}

#define ENABLE_testNormalMessages 1
#if !ENABLE_testNormalMessages
#pragma message ("ENABLE_testNormalMessages " __FILE__)
#else // ENABLE_testNormalMessages
BOOST_AUTO_TEST_CASE(testNormalMessages)
{
	BOOST_CHECK_LE(sizeof(Message), CacheLineSize);
    static const size_t messageSize = sizeof(alphabet);
    static const size_t messageCount = 2;
 
    auto bytesNeeded = HQMemoryBlockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) HQMemoryBlockPool(bytesNeeded, messageSize);

    Message message1(pool);
    BOOST_CHECK_EQUAL(message1.getUsed(), 0u);
    BOOST_CHECK(message1.isEmpty());
    BOOST_CHECK_EQUAL(message1.getType(), Message::Unused);
    message1.setType(Message::LocalType0);

    message1.appendBinaryCopy(alphabet.data(), letterCount);
    BOOST_CHECK_EQUAL(message1.getUsed(), letterCount);
    BOOST_CHECK(!message1.isEmpty());

    BOOST_CHECK_EQUAL(message1.get(), message1.getConst());
    BOOST_CHECK(message1.needAvailable(letterCount));
    BOOST_CHECK(!message1.needAvailable(message1.available() + 1));

    std::string value1(message1.get<char>(), message1.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0, letterCount), value1);

    Message message2(pool);
    message2.appendBinaryCopy(alphabet.data() + letterCount, letterCount);

    BOOST_CHECK_EQUAL(message2.getUsed(), letterCount);
    BOOST_CHECK(!message2.isEmpty());

    std::string value2(message2.get<char>(), message2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(letterCount), value2);

    message1.moveTo(message2);

    BOOST_CHECK_EQUAL(message1.getType(), Message::LocalType0);
    BOOST_CHECK_EQUAL(message1.getUsed(), 0U);
    BOOST_CHECK(message1.isEmpty());
    BOOST_CHECK(message1.needAvailable(2 * letterCount));

    BOOST_CHECK_EQUAL(message2.getType(), Message::LocalType0);
    BOOST_CHECK_EQUAL(message2.getUsed(), letterCount);
    BOOST_CHECK(!message2.isEmpty());

    std::string value2c(message2.get<char>(), message2.getUsed());
    BOOST_CHECK_EQUAL(alphabet.substr(0,letterCount), value2c);

    message1.release();
    BOOST_CHECK_EQUAL(message1.getUsed(), 0U);
    BOOST_CHECK_EQUAL(message1.getOffset(), 0U);
    BOOST_CHECK(message1.isEmpty());
    BOOST_CHECK(! message1.getContainer());
    BOOST_CHECK_EQUAL(message1.available(), 0U);
    BOOST_CHECK(!message1.needAvailable(1));
}
#endif // ENABLE_testNormalMessages

#define DISABLE_testMessageAppendz
#ifdef DISABLE_testMessageAppend
#pragma message ("DISABLE_testMessageAppend " __FILE__)
#else // DISABLE DISABLE_testMessageAppend
BOOST_AUTO_TEST_CASE(testMessageAppend)
{
    static const size_t messageSize = alphabet.size();
    static const size_t messageCount = 2;
    auto bytesNeeded = HQMemoryBlockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[bytesNeeded]);
    auto pool = new (block.get()) HQMemoryBlockPool(bytesNeeded, messageSize);

    Message message(pool);

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
    message.appendCopy(lowerCase);
    message.appendCopy(upperCase);
    std::string value2(message.get<char>(), message.getUsed());
    BOOST_CHECK_EQUAL(alphabet, value2);
}
#endif // DISABLE_testMessageAppend

/* TO BE TESTED
Message sequence, timestamp, type
*/

