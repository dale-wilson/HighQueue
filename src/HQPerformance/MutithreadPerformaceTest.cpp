#include <Common/HighQueuePch.hpp>
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.hpp>
#include <HighQueue/Consumer.hpp>
#include <Common/Stopwatch.hpp>
#include <Mocks/MockMessage.hpp>

using namespace HighQueue;
typedef MockMessage<13> ActualMessage;

namespace
{
    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;

    void producerFunction(ConnectionPtr & connection, uint32_t producerNumber, uint64_t messageCount, std::ostream & stats)
    {
        try
        {
            Producer producer(connection);
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

    void displayResults(
            std::ostream & out,  
            size_t producerCount,
            size_t messageCount,
            size_t messageBytes,
            uint64_t lapse,
            bool & header
        )
    {
        if(header)
        {
            header = false;
            out << "Producers"
                << "\tMessages"
                << "\tBytes/message"
                << "\tSeconds"
                << "\tNanosecond/message"
                << "\tM message/second"
                << std::endl;
        }
        out << producerCount
            << '\t' << messageCount 
            << '\t' << messageBytes
            << '\t' << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond)
            << '\t' << lapse / messageCount;
        if(lapse == 0)
        {
            out << "\t0\tRun time too short to measure.   Use a larger messageCount" << std::endl;
        }
        else
        {
            out << '\t' << std::setprecision(3) << double(messageCount * 1000) / double(lapse)
            << std::endl;
        }
    }

    bool needHeader = true;
} // namespace

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

    connection->willProduce(); // enable solo mode
    Producer producer(connection);
    Consumer consumer(connection);
    Message producerMessage(connection);
    Message consumerMessage(connection);

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
    
    displayResults(
        std::cout,
        0,
        messageCount,
        sizeof(ActualMessage),
        lapse,
        needHeader);

    std::cerr << "Producer: ";
    producer.writeStats(std::cerr);

    std::cerr << "Consumer: ";
    consumer.writeStats(std::cerr);
    std::cerr << std::endl;

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
    static const size_t baseNumberOfProducers = coreCount == 1 ? coreCount : coreCount -1; // Performance drops off severely when competing for cores.
                                                                                   // This is worth measauring, but not everytime.
                                                                                   // You can see the beginning of the effect using this number because
                                                                                   // the threads start competing with Windows itself for the last core.
    static const size_t extraProducers = 9;
    static const size_t maxNumberOfProducers = baseNumberOfProducers + extraProducers;                                                                                
    static const size_t numberOfConsumers = 1;  // Just for documentation
    static const size_t messageCount = entryCount + numberOfConsumers +  maxNumberOfProducers;

    static const size_t consumerSpinCount = 0;
    static const size_t consumerYieldCount = 0;
    static const size_t consumerSleepCount = WaitStrategy::FOREVER;
    static const size_t producerSpinCount = 10;
    static const size_t producerYieldCount = 1000;
    static const size_t producerSleepCount = WaitStrategy::FOREVER;
    static const auto sleepTime = std::chrono::nanoseconds(10);
    
    WaitStrategy consumerStrategy(consumerSpinCount, consumerYieldCount, consumerSleepCount, sleepTime);
    WaitStrategy producerStrategy(producerSpinCount, producerYieldCount, producerSleepCount, sleepTime);
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

        /// solo first time thru.  After that, no-solo.
        connection->willProduce();

        for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
        {
            producerThreads.emplace_back(
                std::bind(producerFunction, connection, nTh, perProducer, std::ref(stats[nTh])));
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

        displayResults(
            std::cout,
            producerCount,
            actualMessageCount,
            sizeof(ActualMessage),
            lapse,
            needHeader);

        for(auto & out : stats)
        {
            std::cerr << "Producer: " << out.str();
        }
        std::cerr << "Consumer: ";
        consumer.writeStats(std::cerr);
        std::cerr << std::endl;
    }
}
#endif // ENABLE_MultithreadMessagePassingPerformance
