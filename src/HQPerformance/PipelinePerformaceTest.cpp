#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>
#include <HighQueue/Consumer.h>
#include <Common/Stopwatch.h>
#include <Mocks/TestMessage.h>


using namespace HighQueue;
#define USE_PRONGHORN_MESSAGE 1
#define VALIDATE_OUTPUT 0
namespace
{
    typedef TestMessage<13> ActualMessage;
#if USE_PRONGHORN_MESSAGE
    byte_t testArray[] = 
#if 1
    "0123456789ABCDEFGHIJKLMNOHighQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@";// this is Pronghorn's test message
#elif 0
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:,.-_+()*@@@@@@@@@@@@@@";
#else
    "";
#endif
    auto messageBytes = sizeof(testArray);
#else // USE_PRONGHORN_MESSAGE
    auto messageBytes = sizeof(ActualMessage);
#endif // USE_PRONGHORN_MESSAGE

    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;

    void producerFunction(ConnectionPtr & connection, uint32_t producerNumber, uint32_t messageCount)
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
#if USE_PRONGHORN_MESSAGE
                producerMessage.appendBinaryCopy(testArray, sizeof(testArray));
#else // USE_PRONGHORN_MESSAGE
                auto testMessage = producerMessage.emplace<ActualMessage>(producerNumber, messageNumber);
#endif //USE_PRONGHORN_MESSAGE
                producer.publish(producerMessage);
            }
            // send an empty message
            producerMessage.setUsed(0);
            producer.publish(producerMessage);
        }
        catch(const std::exception & ex)
        {
            std::cout << "Producer Number " << producerNumber << "Failed " << ex.what() << std::endl;
        }
    }

    enum CopyType
    {
        PassThru,
        BufferSwap,
        BinaryCopy,
        CopyConstruct
    };

    void copyFunction(ConnectionPtr & inConnection, ConnectionPtr & outConnection, CopyType copyType)
    {
        try
        {
            Consumer consumer(inConnection);
            Message consumerMessage(inConnection);
 
            Producer producer(outConnection, true);
            Message producerMessage(outConnection);
            ++threadsReady;
            switch(copyType)
            {
                case PassThru:
                {
                    while(consumer.getNext(consumerMessage))
                    {
                        auto used = consumerMessage.getUsed();
                        producer.publish(consumerMessage);
                        if(used == 0)
                        {
                            return;
                        }
                    }
                    break;
                }
                case BufferSwap:
                    while(consumer.getNext(consumerMessage))
                    {
                        auto used = consumerMessage.getUsed();
                        consumerMessage.moveTo(producerMessage);
                        producer.publish(producerMessage);
                        consumerMessage.destroy<ActualMessage>();
                        if(used == 0)
                        {
                            return;
                        }
                    }
                    break;
                case BinaryCopy:
                {
                    while(consumer.getNext(consumerMessage))
                    {
                        auto used = consumerMessage.getUsed();
                        producerMessage.appendBinaryCopy(consumerMessage.get(), used);
                        producer.publish(producerMessage);
                        if(used == 0)
                        {
                            return;
                        }
                    }
                    break;
                }
                case CopyConstruct:
                {
                    {
                        while(consumer.getNext(consumerMessage))
                        {
                            auto used = consumerMessage.getUsed();
                            if(used == 0)
                            {
                                producerMessage.setUsed(0);
                                producer.publish(producerMessage);
                                return;
                            }
                            producerMessage.emplace<ActualMessage>(*consumerMessage.get<ActualMessage>());
                            producer.publish(producerMessage);
                            consumerMessage.destroy<ActualMessage>();
                        }
                        break;
                    }
                }

            }
        }
        catch(const std::exception & ex)
        {
            std::cout << "Copy thread failed. " << ex.what() << std::endl;
        }
    }
}

#define ENABLE_ARBITRATORPERFORMANCE 1
#if ENABLE_ARBITRATORPERFORMANCE
BOOST_AUTO_TEST_CASE(testPipelinePerformance)
{
    static const size_t numberOfConsumers = 1;   // Don't change this
    static const size_t maxNumberOfProducers = 1;   // Don't change this

    static const size_t copyLimit = 1;       // This you can change.

    static const size_t entryCount = 10000;
    static const uint32_t targetMessageCount = 100000000; //3000000;

    static const size_t queueCount = copyLimit + numberOfConsumers; // need a pool for each object that can receive messages

    // how many buffers do we need?
    static const size_t messageCount = entryCount * queueCount + numberOfConsumers + 2 * copyLimit + maxNumberOfProducers;

    static const size_t spinCount = 0;
    static const size_t yieldCount = 0;//1;//100;
    static const size_t sleepCount = WaitStrategy::FOREVER;
    static const std::chrono::nanoseconds sleepPeriod(2);
    CopyType copyType = BinaryCopy;
                      // BufferSwap;
                      // BinaryCopy;

    std::cout << "Pipeline " << (maxNumberOfProducers + copyLimit + numberOfConsumers) << " stage. Copy type: " << copyType << ": ";

    WaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepPeriod);
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageBytes);
    MemoryPoolPtr memoryPool(new MemoryPool(messageBytes, messageCount));

    std::vector<std::shared_ptr<Connection> > connections;
    for(size_t nConn = 0; nConn < copyLimit + numberOfConsumers; ++nConn)
    {
        std::shared_ptr<Connection> connection(new Connection);
        connections.push_back(connection);
        std::stringstream name;
        name << "Connection " << nConn;
        connection->createLocal(name.str(), parameters, memoryPool);
    }

    // The consumer listens to the last connection
    Consumer consumer(connections.back());
    Message consumerMessage(connections.back());
    
    producerGo = false;
    threadsReady = 0;
    uint64_t nextMessage = 0u;

    // Each copy thread listens to connection N-1 and sends to thread N
    std::vector<std::thread> threads;
    for(size_t nCopy = connections.size() - 1; nCopy > 0; --nCopy)
    {
        threads.emplace_back(std::bind(copyFunction,
            connections[nCopy - 1],
            connections[nCopy],
            copyType)
            );
    }

    // The producer sends targetMessageCount messages to connection 0
    threads.emplace_back(
        std::bind(producerFunction, connections[0], 1, targetMessageCount));
 
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
#if VALIDATE_OUTPUT 
        auto testMessage = consumerMessage.read<ActualMessage>();
        testMessage->touch();
        if(nextMessage != testMessage->getSequence())
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(nextMessage, testMessage->getSequence());
        }
        consumerMessage.destroy<ActualMessage>();
        ++ nextMessage;
#endif // VALIDATE_OUTPUT
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

    consumer.writeStats(std::cout);
}
#endif // ENABLEARBITRATORPERFORMANCE
