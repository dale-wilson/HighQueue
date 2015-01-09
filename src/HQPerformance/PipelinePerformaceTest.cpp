#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>
#include <HighQueue/Consumer.h>
#include <Common/Stopwatch.h>
#include <HQPerformance/TestMessage.h>


using namespace HighQueue;
#define MATCH_PRONGHORN 0
namespace
{
    typedef TestMessage<13> ActualMessage;
#if MATCH_PRONGHORN
    byte_t testArray[] = 
#if 1
    "0123456789ABCDEFGHIJKLMNOHighQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@";// this is Pronghorn's test message
#elif 0
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@";
#else
    "";
#endif
    auto messageBytes = sizeof(testArray);
#else // MATCH_PRONGHORN
    auto messageBytes = sizeof(ActualMessage);
#endif // MATCH_PRONGHORN

    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;

    void producerFunction(Connection & connection, uint32_t producerNumber, uint32_t messageCount)
    {
        try
        {
            Producer producer(connection, true);
            Message producerMessage(connection);

            ++threadsReady;
            while(!producerGo)
            {
                std::this_thread::yield();
            }

            for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
            {
#if MATCH_PRONGHORN
                producerMessage.appendBinaryCopy(testArray, sizeof(testArray));
#else // MATCH_PRONGHORN
                auto testMessage = producerMessage.emplace<ActualMessage>(producerNumber, messageNumber);
#endif //MATCH_PRONGHORN
                producer.publish(producerMessage);
            }
            // send an empty message
            producerMessage.setUsed(0);
            producer.publish(producerMessage);
        }
        catch(const std::exception & ex)
        {
            std::cerr << "Producer Number " << producerNumber << "Failed " << ex.what() << std::endl;
        }
    }

    void copyFunction(Connection & inConnection, Connection & outConnection, bool passThru)
    {
        try
        {
            Consumer consumer(inConnection);
            Message consumerMessage(inConnection);
 
            Producer producer(outConnection, true);
            Message producerMessage(outConnection);
            ++threadsReady;
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
#if MATCH_PRONGHORN
                    producerMessage.appendBinaryCopy(consumerMessage.get(), used);
#else // MATCH_PRONGHORN
                    producerMessage.emplace<ActualMessage>(*consumerMessage.get<ActualMessage>());
#endif // MATCH_PRONGHORN
                    producer.publish(producerMessage);
                    consumerMessage.destroy<ActualMessage>();
                }
            }
        }
        catch(const std::exception & ex)
        {
            std::cerr << "Copy thread failed. " << ex.what() << std::endl;
        }
    }
}

BOOST_AUTO_TEST_CASE(testPipelinePerformance)
{
    static const size_t consumerLimit = 1;   // Don't change this
    static const size_t producerLimit = 1;   // Don't change this

    static const size_t copyLimit = 1;       // This you can change.

    static const size_t entryCount = 40000;
    static const uint32_t targetMessageCount = 3000000; 

    // how many buffers do we need?
    static const size_t messageCount = entryCount + consumerLimit + copyLimit + producerLimit;

    static const size_t spinCount = 10000;
    static const size_t yieldCount = ConsumerWaitStrategy::FOREVER;
    bool passThru = false;

    std::cerr << "Pipeline " << (producerLimit + copyLimit + consumerLimit) << (passThru?"+":"") << " stage: ";

    ConsumerWaitStrategy strategy(spinCount, yieldCount);
    CreationParameters parameters(strategy, entryCount, messageBytes, messageCount);

    std::vector<std::shared_ptr<Connection> > connections;
    for(size_t nConn = 0; nConn < copyLimit + consumerLimit; ++nConn)
    {
        std::shared_ptr<Connection> connection(new Connection);
        connections.push_back(connection);
        std::stringstream name;
        name << "Connection " << nConn;
        connection->createLocal(name.str(), parameters);
    }

    // The consumer listens to the last connection
    Consumer consumer(*connections.back());
    Message consumerMessage(*connections.back());
    
    producerGo = false;
    threadsReady = 0;
    uint64_t nextMessage = 0u;

    // Each copy thread listens to connection N-1 and sends to thread N
    std::vector<std::thread> threads;
    for(size_t nCopy = connections.size() - 1; nCopy > 0; --nCopy)
    {
        threads.emplace_back(std::bind(copyFunction,
            std::ref(*connections[nCopy - 1]),
            std::ref(*connections[nCopy]),
            passThru)
            );
    }

    // The producer sends targetMessageCount messages to connection 0
    threads.emplace_back(
        std::bind(producerFunction, std::ref(*connections[0]), 1, targetMessageCount));
 
    // All wired up, ready to go.  Wait for the threads to initialize.
    while(threadsReady < threads.size())
    {
        std::this_thread::yield();
    }

    Stopwatch timer;
    producerGo = true;

    for(uint64_t messageNumber = 0; messageNumber < targetMessageCount; ++messageNumber)
    {
        consumer.getNext(consumerMessage);
#if MATCH_PRONGHORN 
        // Pronghorn final stage ignores incoming data.
#else // MATCH_PRONGHORN
        auto testMessage = consumerMessage.get<ActualMessage>();
        testMessage->touch();
        if(nextMessage != testMessage->messageNumber())
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(nextMessage, testMessage->messageNumber());
        }
        consumerMessage.destroy<ActualMessage>();
        ++ nextMessage;
#endif // MATCH_PRONGHORN
    }

    auto lapse = timer.nanoseconds();
    
    for(auto & thread: threads)
    {
        thread.join();
    }

    auto messageBits = messageBytes * 8;

    std::cout << " Passed " << targetMessageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  " 
        << lapse / targetMessageCount << " nsec./message "
        << std::setprecision(3) << double(targetMessageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(targetMessageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(targetMessageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;

    // for connections that may share messages (i.e. passThru), close them all before any go out of scope
    // to avoid releasing buffers into pools that have been deleted.
    for (const auto & connection : connections)
    {
        connection->close();
    }
}
