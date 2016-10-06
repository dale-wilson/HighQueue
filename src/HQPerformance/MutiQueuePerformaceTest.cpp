#include <Common/HighQueuePch.hpp>
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.hpp>
#include <HighQueue/MultiQueueConsumer.hpp>
#include <Common/Stopwatch.hpp>
#include <Mocks/MockMessage.hpp>

using namespace HighQueue;
typedef MockMessage<13> ActualMessage;

namespace
{
    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;
    bool needHeader = true;

    void producerFunction(ConnectionPtr & connection, uint32_t producerNumber, uint64_t messageCount, std::ostream & stats)
    {
        try
        {
            Producer producer(connection);
            Message producerMessage(connection);

            stats << "Producer: #" << producerNumber << " ";

            ++threadsReady;
            while(!producerGo)
            {
                std::this_thread::yield();
            }

            Stopwatch timer;
            for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
            {
                auto testMessage = producerMessage.emplace<ActualMessage>(producerNumber, messageNumber);
                producer.publish(producerMessage);
            }
            auto lapse = timer.nanoseconds();
            stats << lapse << " nanoseconds.  " << lapse / messageCount << " nanoseconds/message.  ";
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
            out << std::setw(6) << "Queues" << '\t' 
                << std::setw(10) << "Producers" << '\t' 
                << std::setw(10) << "Messages" << '\t' 
                << std::setw(14) << "Bytes/message" << '\t' 
                << std::setw(10) << "Seconds" << '\t' 
                << std::setw(18) << "Nanosecond/message" << '\t' 
                << std::setw(16) << "M message/second" << std::endl;
        }
        out << std::setw(6) << producerCount << '\t' 
            << std::setw(10) << producerCount << '\t' 
            << std::setw(10) << messageCount << '\t' 
            << std::setw(14) << messageBytes << '\t' 
            << std::setw(10) << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << '\t' 
            << std::setw(18) << lapse / messageCount << '\t';
        if(lapse == 0)
        {
            out << "0\tRun time too short to measure.   Use a larger messageCount" 
                << std::endl;
        }
        else
        {
            out << std::setw(16) << std::setprecision(3) << double(messageCount * 1000) / double(lapse)
                << std::endl;
        }
    }

    void runMultiQueueTest(size_t producerCount)
    {                                                                                 
        const uint64_t targetMessageCount = 1000000 * 100; // runs about 5 to 10 seconds in release/optimized build
        const size_t messageSize = sizeof(ActualMessage);

        const size_t entryCount = 100000;
        const size_t consumerCount = 1;  // Just for documentation
        const size_t queueCount = producerCount;

        // Each queue  holds entryCount Messages
        // each producer has one Message
        // each consumer has one Message
        const size_t messageCount = entryCount * queueCount + producerCount + consumerCount;

        // Fine tune the Consumer delay
        const auto sleepTime = std::chrono::nanoseconds(10);
        const size_t consumerSpinCount = 100;
        const size_t consumerYieldCount = 1000;
        const size_t consumerSleepCount = WaitStrategy::FOREVER;
        WaitStrategy consumerStrategy(consumerSpinCount,consumerYieldCount,consumerSleepCount,sleepTime);

        // Fine tune the Producer delay
        const size_t producerSpinCount = 10;
        const size_t producerYieldCount = 1000;
        const size_t producerSleepCount = WaitStrategy::FOREVER;
        WaitStrategy producerStrategy(producerSpinCount, producerYieldCount, producerSleepCount, sleepTime);

        bool discardMessagesIfNoConsumer = false;
        CreationParameters parameters(producerStrategy, consumerStrategy, discardMessagesIfNoConsumer, entryCount, messageSize, messageCount);

        // Explicitly construct a MemoryPool to be shared among all producers and the consumer
        MemoryPoolPtr memoryPool = std::make_shared<MemoryPool>(messageSize, messageCount);

        MultiQueueConsumer consumer;
        std::vector<ConnectionPtr> connections;
        std::vector<std::thread> producerThreads;
        for(size_t connectionCount = 0; connectionCount < producerCount; ++connectionCount)
        {
            // Each producer has its own queue meaning its own Connection
            ConnectionPtr connection = std::make_shared<Connection>();
            connection->createLocal("Local", parameters, memoryPool);
            // each individual connection is solo
            connection->willProduce();
            connections.push_back(connection);
            consumer.addQueue(connection);
        }

        // Allocate a message for use by the producer
        Message consumerMessage(memoryPool);

        // Track message sequence numbers from each producer
        std::vector<uint64_t> nextMessage(producerCount, 0ULL);
        // accumulate statistics for each producer.
        std::vector<std::stringstream> stats(producerCount);

        // Divide up the messages-to-be-produced equally among all the producers.
        size_t perProducer = targetMessageCount / producerCount;
        size_t actualMessageCount = perProducer * producerCount;

        // Start up carefully to avoid measuring set-up time.
        threadsReady = 0;
        producerGo = false;

        for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
        {
            producerThreads.emplace_back(
                std::bind(producerFunction, connections[nTh], nTh, perProducer, std::ref(stats[nTh])));
        }
        std::this_thread::yield();

        // wait until all threads have started and are ready to produce.
        while(threadsReady < producerCount)
        {
            std::this_thread::yield();
        }

        // Start the timeer and turn on the stopwatch
        Stopwatch timer;
        producerGo = true;

        // This thread acts as the consumer
        for(uint64_t messageNumber = 0; messageNumber < actualMessageCount; ++messageNumber)
        {
            consumer.getNext(consumerMessage);
            auto testMessage = consumerMessage.get<ActualMessage>();
            
            // Touch the message to keep the cache honest.
            testMessage->touch();

            // verify that messages from a particular producer arrive in the correct order.
            auto & msgNumber = nextMessage[testMessage->producerNumber()];
            if(msgNumber != testMessage->getSequence())
            {
                // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
                BOOST_CHECK_EQUAL(messageNumber, testMessage->getSequence());
            }
            ++ msgNumber; 
        }

        // How long did it take us to consume all the messages?
        auto lapse = timer.nanoseconds();

        // sometimes synchronize thread shut down.
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
            std::cerr << out.str();
        }
        std::cerr << "Consumer:\n";
        consumer.writeStats(std::cerr);
        std::cerr << std::endl;
    }
} // namespace


#define ENABLE_MultiQueueMessagePassingPerformance 1
#if ! ENABLE_MultiQueueMessagePassingPerformance
#pragma message ("ENABLE_MultiQueueMessagePassingPerformance")
#else // ENABLE_MultiQueueMessagePassingPerformance 
BOOST_AUTO_TEST_CASE(testMultitQueueMessagePassingPerformance)
{
    static const size_t coreCount = std::thread::hardware_concurrency();
    static const size_t baseNumberOfProducers = coreCount == 1 ? coreCount : coreCount -1; // Performance drops off severely when competing for cores.
                                                                                           // This is worth measauring, but not everytime.
                                                                                           // You can see the beginning of the effect using this number because
                                                                                           // the threads start competing with Windows itself for the last core.
    static const size_t extraProducers = 9;
    static const size_t maxNumberOfProducers = baseNumberOfProducers + extraProducers;

    std::cerr << "***** BEGIN MultiQueueMessagePassingPerformance test *****" << std::endl;

    for(size_t producerCount = 1; producerCount < maxNumberOfProducers; ++producerCount)
    {
        runMultiQueueTest(producerCount);
    }

    std::cerr << "***** END MultiQueueMessagePassingPerformance test *****" << std::endl;
}

#endif // ENABLE_MultiQueueMessagePassingPerformance
