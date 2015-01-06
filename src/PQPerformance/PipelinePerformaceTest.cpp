#include "Common/ProntoQueuePch.h"
#define BOOST_TEST_NO_MAIN ProntoQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <ProntoQueue/Producer.h>
#include <ProntoQueue/Consumer.h>
#include <Common/Stopwatch.h>
#include <PQPerformance/TestMessage.h>

using namespace ProntoQueue;
#define MATCH_PRONGHORN
namespace
{
    byte_t testArray[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@";//, this is a reasonable test message.".getBytes();

    volatile std::atomic<uint32_t> producersWaiting;
    volatile bool producersGo = false;

    void producerFunction(Connection & connection, uint32_t producerNumber, uint64_t messageCount, bool solo)
    {
        Producer producer(connection, solo);
        ProntoQueue::Message producerMessage;
        if(!connection.allocate(producerMessage))
        {
            std::cerr << "Failed to allocate message for producer Number " << producerNumber << std::endl;
            return;
        }

        ++producersWaiting;
        while(!producersGo)
        {
            std::this_thread::yield();
        }

        for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
        {
#ifdef MATCH_PRONGHORN
            producerMessage.appendBinaryCopy(testArray, sizeof(testArray));
#else // MATCH_PRONGHORN
            auto testMessage = producerMessage.construct<TestMessage>(producerNumber, messageNumber);
#endif //MATCH_PRONGHORN
            producer.publish(producerMessage);
        }
        // send an empty message
        producerMessage.setUsed(0);
        producer.publish(producerMessage);
    }

    void copyFunction(Connection & inConnection, Connection & outConnection, bool passThru)
    {
        Consumer consumer(inConnection);
        ProntoQueue::Message consumerMessage;
        if(!inConnection.allocate(consumerMessage))
        {
            std::cerr << "Failed to allocate consumer message for copy thread." << std::endl;
            return;
        }

        Producer producer(outConnection, true);
        ProntoQueue::Message producerMessage;
        if(!outConnection.allocate(producerMessage))
        {
            std::cerr << "Failed to allocate producer message for copy thread." << std::endl;
            return;
        }
        if(passThru)
        {
            while(true)
            {
                consumer.getNext(consumerMessage);
                auto used = consumerMessage.getUsed();
                producer.publish(consumerMessage);
                if(used == 0)
                {
                    return;
                }
            }
        }
        else
        {
            while(true)
            {
                consumer.getNext(consumerMessage);
                auto used = consumerMessage.getUsed();
                if(used == 0)
                {
                    producerMessage.setUsed(0);
                    producer.publish(producerMessage);
                    return;
                }
#ifdef MATCH_PRONGHORN
                producerMessage.appendBinaryCopy(consumerMessage.get(), used);
#else // MATCH_PRONGHORN
                producerMessage.construct<TestMessage>(*consumerMessage.get<TestMessage>());
#endif // MATCH_PRONGHORN
                producer.publish(producerMessage);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testPipelinePerformance)
{
    static const size_t consumerLimit = 1;
    static const size_t copyLimit = 1;
    static const size_t producerLimit = 1;

    static const size_t entryCount = 10000;
    static const size_t messageSize = sizeof(TestMessage);
    static const uint64_t targetMessageCount = 3000000; 

    // how many buffers do we need?
    static const size_t messageCount = entryCount + consumerLimit + copyLimit + producerLimit;

    static const size_t spinCount = 10000;
    static const size_t yieldCount = ConsumerWaitStrategy::FOREVER;
    bool passThru = false;

    std::cerr << "Pipeline (3" << (passThru?"+":"") << "): ";

    ConsumerWaitStrategy strategy(spinCount, yieldCount);
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection producerConnection;
    producerConnection.createLocal("Producer", parameters);

    Connection consumerConnection;
    consumerConnection.createLocal("Consuemr", parameters);

    Consumer consumer(consumerConnection);
    ProntoQueue::Message consumerMessage;
    BOOST_REQUIRE(consumerConnection.allocate(consumerMessage));


    auto copyThread = std::thread(std::bind(copyFunction,
        std::ref(producerConnection),
        std::ref(consumerConnection),
        passThru)
        );

    std::vector<std::thread> producerThreads;
    std::vector<uint64_t> nextMessage;

    producersWaiting = 0;
    producersGo = false;

    size_t producerCount = 1; // elsewhere this is a loop index
    size_t perProducer = targetMessageCount / producerCount;
    size_t actualMessageCount = perProducer * producerCount;

    for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
    {
        nextMessage.emplace_back(0u);
        producerThreads.emplace_back(
            std::bind(producerFunction, std::ref(producerConnection), nTh, perProducer, producerCount == 1));
    }

    // All wired up, ready to go.  Wait for the producers to initialize.
    // note the assumption that the copy thread is ready.
    while(producersWaiting < producerCount)
    {
        std::this_thread::yield();
    }

    Stopwatch timer;
    producersGo = true;

    for(uint64_t messageNumber = 0; messageNumber < actualMessageCount; ++messageNumber)
    {
        consumer.getNext(consumerMessage);
#ifdef MATCH_PRONGHORN 
        // Pronghorn final stage ignores incoming data.
#else // MATCH_PRONGHORN
        auto testMessage = consumerMessage.get<TestMessage>();
        testMessage->touch();
        auto producerNumber = testMessage->producerNumber_;
        auto & msgNumber = nextMessage[producerNumber];
        if(msgNumber != testMessage->messageNumber_)
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->messageNumber_);
        }
        ++ msgNumber; 
#endif // MATCH_PRONGHORN
    }

    auto lapse = timer.nanoseconds();

    // sometimes we synchronize thread shut down.
    producersGo = false;

    for(size_t nTh = 0; nTh < producerCount; ++nTh)
    {
        producerThreads[nTh].join();
    }
    copyThread.join();

#ifdef MATCH_PRONGHORN
    auto messageBytes = sizeof(testArray);
#else // MATCH_PRONGHORN
    auto messageBytes = sizeof(TestMessage);
#endif // MATCH_PRONGHORN
    auto messageBits = messageBytes * 8;

    std::cout << " Passed " << actualMessageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  " 
        << lapse / actualMessageCount << " nsec./message "
        << std::setprecision(3) << double(actualMessageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(actualMessageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(actualMessageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;

    // for connections that may share buffers, close them before they go out of scope.
    consumerConnection.close();
    producerConnection.close();
}
