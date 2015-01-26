#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>
#include <HighQueue/Consumer.h>
#include <Common/Stopwatch.h>
#include <Mocks/TestMessage.h>

using namespace HighQueue;
typedef TestMessage<13> ActualMessage;

namespace
{
    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;

    void producerFunction(ConnectionPtr & connection, uint32_t producerNumber, uint64_t messageCount, bool solo, std::ostream & stats)
    {
        try
        {
            Producer producer(connection, solo);
            Message producerMessage(connection);
        
            ++threadsReady;
            while(!producerGo)
            {
                std::this_thread::yield();
            }

            for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
            {
                auto testMessage = producerMessage.emplace<ActualMessage>(producerNumber, messageNumber);
                producer.publish(producerMessage);
            }
            producer.writeStats(stats);
        }
        catch(const std::exception & ex)
        {
            std::cout << "Producer Number " << producerNumber << " failed. " << ex.what() << std::endl;
            return;
        }
    }
}


#define ENABLE_ST_PERFORMANCE 1
#if ENABLE_ST_PERFORMANCE
BOOST_AUTO_TEST_CASE(testSingleThreadedMessagePassingPerformance)
{
    WaitStrategy strategy;
    size_t entryCount = 262144 / 2; // <- thats the number of messages in the primaryRingBuffer in the pronghorn test //100000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    uint64_t messageCount = 1000000 * 100;

    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messagesNeeded);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

	Producer producer(connection, false);
    Consumer consumer(connection);
    Message producerMessage(connection);
    Message consumerMessage(connection);

    std::cout << "Single threaded: ";

    Stopwatch timer;

    for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
    {
        producerMessage.emplace<ActualMessage>(1, messageNumber);
        producer.publish(producerMessage);
        consumer.getNext(consumerMessage);
        auto testMessage = consumerMessage.get<ActualMessage>();
        if(messageNumber != testMessage->getSequence())
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->getSequence());
        }
    }
    auto lapse = timer.nanoseconds();
    auto messageBytes = sizeof(ActualMessage);
    auto messageBits = sizeof(ActualMessage) * 8;
    std::cout << "Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(3) << double(messageCount * 1000) / double(lapse) << " MMsg/second "
#if defined(DISPLAY_PRONGHORN_STYLE_RESULTS)
        << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
#endif //DISPLAY_PRONGHORN_STYLE_RESULTS
        << std::endl;

	std::cout << "Single thread Statistics" << std::endl;
	std::cout << "Producer: ";
	producer.writeStats(std::cout);

	std::cout << "Consumer: ";
	consumer.writeStats(std::cout);
	std::cout << std::endl;

}
#endif // ENABLEST_PERFORMANCE


#define ENABLE_MultithreadMessagePassingPerformance 1
#if ! ENABLE_MultithreadMessagePassingPerformance
#pragma message ("ENABLE_MultithreadMessagePassingPerformance")
#else // ENABLE_MultithreadMessagePassingPerformance 
BOOST_AUTO_TEST_CASE(testMultithreadMessagePassingPerformance)
{
    static const size_t entryCount = 100000;
    static const size_t messageSize = sizeof(ActualMessage);

    static const uint64_t targetMessageCount = 1000000 * 100; // runs about 5 to 10 seconds in release/optimized build
    static const size_t coreCount = std::thread::hardware_concurrency();
    static const size_t maxNumberOfProducers = coreCount == 1 ? coreCount : coreCount -1; // Performance drops off severely when competing for cores.
                                                                                   // This is worth measauring, but not everytime.
                                                                                   // You can see the beginning of the effect using this number because
                                                                                   // the threads start competing with Windows itself for the last core.
    static const size_t numberOfConsumers = 1;  // Just for documentation
    static const size_t messageCount = entryCount + numberOfConsumers +  maxNumberOfProducers;

    static const size_t spinCount = 0;
    static const size_t yieldCount = 0;
    static const size_t sleepCount = WaitStrategy::FOREVER;
    static const auto sleepTime = std::chrono::nanoseconds(10);
    
    WaitStrategy consumerStrategy(spinCount, yieldCount, sleepCount, sleepTime);
    WaitStrategy producerStrategy(spinCount, yieldCount + 10000, sleepCount, sleepTime);
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(producerStrategy, consumerStrategy, discardMessagesIfNoConsumer, entryCount, messageSize, messageCount);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

    Consumer consumer(connection);
    Message consumerMessage(connection);

    for(size_t producerCount = 1; producerCount <= maxNumberOfProducers; ++producerCount)
    {
        std::vector<std::thread> producerThreads;
        std::vector<uint64_t> nextMessage(producerCount, 0ULL);
        std::vector<std::stringstream> stats(producerCount);

        threadsReady = 0;
        producerGo = false;
        size_t perProducer = targetMessageCount / producerCount;
        size_t actualMessageCount = perProducer * producerCount;

        for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
        {
            producerThreads.emplace_back(
                std::bind(producerFunction, connection, nTh, perProducer, producerCount == 1, std::ref(stats[nTh])));
        }
        std::this_thread::yield();

        while(threadsReady < producerCount)
        {
            std::this_thread::yield();
        }

        Stopwatch timer;
        producerGo = true;

        for(uint64_t messageNumber = 0; messageNumber < actualMessageCount; ++messageNumber)
        {
            consumer.getNext(consumerMessage);
            auto testMessage = consumerMessage.get<ActualMessage>();
            testMessage->touch();
            auto & msgNumber = nextMessage[testMessage->producerNumber()];
            if(msgNumber != testMessage->getSequence())
            {
                // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
                BOOST_CHECK_EQUAL(messageNumber, testMessage->getSequence());
            }
            ++ msgNumber; 
        }

        auto lapse = timer.nanoseconds();

        // sometimes we synchronize thread shut down.
        producerGo = false;

        for(size_t nTh = 0; nTh < producerCount; ++nTh)
        {
            producerThreads[nTh].join();
        }

        auto messageBytes = sizeof(ActualMessage);
        auto messageBits = sizeof(ActualMessage) * 8;
        std::cout << "HighQueue Test " << producerCount << " producer" << std::fixed;
        std::cout << " passed " << actualMessageCount << ' ' << messageBytes << " byte messages in "
            << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  ";
        if(lapse == 0)
        {
            std::cout << "Run time too short to measure.   Use a larger messageCount" << std::endl;
        }
        else
        {
            std::cout
                << lapse / actualMessageCount << " nsec./message "
                << std::setprecision(3) << double(actualMessageCount * 1000) / double(lapse) << " MMsg/second "
#if defined(DISPLAY_PRONGHORN_STYLE_RESULTS)
                << std::setprecision(3) << double(actualMessageCount * messageBytes) / double(lapse) << " GByte/second "
                << std::setprecision(3) << double(actualMessageCount * messageBits) / double(lapse) << " GBit/second."
#endif //DISPLAY_PRONGHORN_STYLE_RESULTS
                << std::endl;
        }

        std::cout << "Multithread Statistics" << std::endl;
        for(auto & out : stats)
        {
            std::cout << "Producer: " << out.str();
        }
        std::cout << "Consumer: ";
        consumer.writeStats(std::cout);
        std::cout << std::endl;
    }
}
#endif // ENABLE_MultithreadMessagePassingPerformance
