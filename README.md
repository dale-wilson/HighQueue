# HighQueue
A Queue for High Speed message passing between threads, processes, or other components.

```
  The HighQueue Haiku

  To send messages
  So quickly you must use a
  Good algorithm.
```

##Introduction
A High Performance Queue (HighQueue) is a mechanism for message passing between components of a system.  It is most often used to pass messages between two or more threads in the same process, but it can also be used for intra-process communication by putting the HighQueue in shared memory. 

###Clients
HighQueue clients can be either a Publisher or a Consumer.
  *	Producers create and publish data into the HighQueue.   
  * Consumers accept and process data from the HighQueue.
  
Each HighQueue instance supports multiple Producers, but only a single Consumer.

##How fast is it?
Absolute performance numbers are notoriously difficult to measure accurately. The results vary widely from machine to machine.

That being said, tests on an Intel(R) Core(TM) i7-4790 CPU @3.60GHz [8 cores] with one producer and one consumer
show sustained throughput is around 16.6 million messages/second.  This translates to around 60 nanoseconds 
per message delivery.  It can sustain this rate up to 6 producers and one consumer.  Beyond that the threads start
competing with the operating system (Win7) and with each other for the available cores and throughput starts to drop.
[With 8 producers + 1 consumer + Windows a message is delivered to the consumer thread every 145 nanoseconds.]

You can generate equivalent numbers for your machine by running the MultithreadPerformanceTest which is included in the HQPerformance executable.

<em>Your Mileage Will Vary!</em>

## The Client Api
The API used by HighQueue Producers and Consumers is designed to simple and to use common C++ idioms.  Experienced C++ programs should be able to get up-to-speed with HighQueue very quickly.
The HighQueue API is described in [this Wiki page.](https://github.com/dale-wilson/HighQueue/wiki/HighQueue-API)
