# HighQueue and HighStages
<b>HighQueue</b> began as tool for High Speed message passing between threads, processes, or other components of a system.   It has grown to include <b>HighStages</b> -- a framework for creating systems based on a pipeline architecture.


```
  The HighQueue Haiku

  Sending messages
  Quickly takes clean code and a
  Good algorithm.
```

##Introduction

HighQueue -- A High Performance Queue -- is a mechanism for message passing between components of a system.  It is most often used to pass messages between two or more threads in the same process, but it can also be 
used for intra-process communication by putting the HighQueue in shared memory. 

HighStages is framework for creating the processing Stages -- the units that can be assembled into a processing pipeline.   HighStages defines tools for developing the Stages.
Eventually it will also provide the support needed to instantiate and configure those Stages, assemble them into a pipeline and run the resulting program.

As of this writing (mid-January, 2015) HighQueue is relatively stable (although its documentation is lagging behind)  

HighStages is still undergoing rapid developemnt and the corresponding rapid changes as it evolves into a complete, stable system.

##Overview 

##HighQueue Clients

A HighQueue client can be either a Publisher or a Consumer.
  *	Producers create and publish data into the HighQueue.   
  * Consumers accept and process data from the HighQueue.
  
Each HighQueue instance (each individual queue) supports multiple Producers, but only a single Consumer.

## The HighQueue Client API

The API used by HighQueue Producers and Consumers is designed to simple and to use common C++ idioms.  Experienced C++ programs should be able to get up-to-speed with HighQueue very quickly.
The HighQueue API is described on [this Wiki page.](https://github.com/dale-wilson/HighQueue/wiki/HighQueue-API)

## The HighStages API
Developers who are creating pipeline-based applications using the HighStages framework can use the HighStages API to create the Stages in this pipeline.

HighStages-based Stages receive messages from previous Stages; apply a transformation to the messages, then pass these messages (or new messages derived from the input messages) 
on to subsequent stages in the pipeline.  The HighStages library supplies the plumbing and the tools.  
It's up to application-specific code supplied by the developer to perform the actual work.

The HighStages API is described on [this Wiki page.](https://github.com/dale-wilson/HighQueue/wiki/HighStages-API)

##How fast is it?
###The short answer:  

Very fast!

###The more detailed answer:

Absolute performance numbers are notoriously difficult to measure accurately. The results vary widely from machine to machine.

That being said, tests on an Intel(R) Core(TM) i7-4790 CPU @3.60GHz [8 cores] I measured these results:

####HighQueue performance in a simple configuration.

With one producer and one consumer HighQueue can pass a message between threads at a sustained rate of 40 million messages per second (that's about 25 nanoseconds per message.)

#####Adding producers.

As the load grows to 6 producers feeding a single consumer the rate drops to around 16 million messages per second (60 nanoseconds per message) because the 
producers are contending for the right to send the next message.  Beyond that the threads start competing with the operating system (Win7) and with each other 
for the available cores and throughput drops more steeply.  With 8 producers + 1 consumer + Windows competing for 8 cores, a message is delivered to the consumer thread every 145 nanoseconds.

Note it would be highly unusual to have more than a couple of producers feeding messages to a single producer, but if the need arises HighQueue can handle it with style.

####HighStages performance.

<em>Because HighStages is very much still under construction, detailed performance numbers are not available, yet, but preliminary results are promising.</em> 
Watch this space for breaking news.

###Don't take my word for it.

You can generate equivalent numbers for your machine by running the the HQPerformance executable which is created as part of building HighQueue.

<b><em>Your Performance Mileage Will Vary!</em></b>

