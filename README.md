# HighQueue and HighSteps
<b>HighQueue</b> began as tool for high speed message passing between threads (or processes, or other components of a system.)   It has grown to include <b>HighSteps</b> -- a framework for creating systems based on a pipeline architecture.

```
  The HighQueue Haiku

  Sending messages
  Quickly takes clean code and a
  Good algorithm.
```

##Introduction

HighQueue -- A <b>High</b> performance <b>Queue</b> -- is a mechanism for message passing between components of a system.  It is most often used to pass messages between two or more threads in the same process, but it can also be 
used for interprocess communication by putting the HighQueue in shared memory. 

HighSteps is framework for creating <b>High</b> performance processing <b>Steps</b>.  Steps are units that can be assembled into a processing pipeline.   HighSteps defines tools for developing the Steps.  When Steps are running within different threads, the messages are passed using a HighQueue.  When they are running within the same thread a direct virtual call is used to pass the message.
It is still under active development.  Eventually it will also provide the support needed to instantiate and configure those Steps, assemble them into a pipeline and run the resulting program.

###Status
HighQueue is stable.
HighSteps is complete.  There is quite a bit of polishing and documentation in progress.

##Overview 

##HighQueue Clients

A HighQueue client can be either a Publisher or a Consumer.
  *	Producers create and publish data into the HighQueue.   
  * Consumers accept and process data from the HighQueue.
  
Each HighQueue instance (each individual queue) supports multiple Producers, but only a single Consumer.

## The HighQueue Client API

The API used by HighQueue Producers and Consumers is designed to simple and to use common C++ idioms.  Experienced C++ programs should be able to get up-to-speed with HighQueue very quickly.
The HighQueue API is described on [this Wiki page.](https://github.com/dale-wilson/HighQueue/wiki/HighQueue-API)

## HighSteps Overview
```
A journey of a thousand megahertz begins with a single Step.
```


## The HighSteps API
Developers who are creating pipeline-based applications using the HighSteps framework can use the HighSteps API to create the Steps in this pipeline.

HighSteps-based Steps receive messages from previous Steps; apply a transformation to the messages, then pass these messages (or new messages derived from the input messages) 
on to subsequent Steps in the pipeline.  

The HighSteps library supplies the plumbing and the tools.  It's up to application-specific code supplied by the developer to perform the actual work.

The HighSteps API is described on [this Wiki page.](https://github.com/dale-wilson/HighQueue/wiki/HighSteps-API)

##How fast is HighQueue?
###The short answer  

Very fast!

###The more detailed answer

Disclaimer: Absolute performance numbers are notoriously difficult to measure accurately. The results vary widely from machine to machine.

That being said, tests on an Intel(R) Core(TM) i7-4790 CPU @3.60GHz [8 cores] the HighqueuePerfmanceTest measured these results:

####HighQueue performance in a simple configuration.

With one producer and one consumer HighQueue can pass a message between threads at a sustained rate of 40 million messages per second (that's about 25 nanoseconds per message.)

#####Adding producers.

With more than one Producer the producers must contend for the right to publish the next message.  This reduces the throughput to around 11 to 14 million messages per second (70 to 90 nanoseconds per message) 

This rate remains stable up to 12 threads (1.5 x the number of cores).  I haven't tested it beyond that.

Note it would be highly unusual to have more than a couple of producers feeding messages to a single producer, but if the need arises HighQueue can handle it with style.

##How fast is HighSteps?

<em>Because HighSteps is very much still under construction, detailed performance numbers are not available, yet, but preliminary results are promising.</em> 
Watch this space for breaking news.

###Don't take my word for it.

You can generate equivalent numbers for your machine by running the the HighQueuePerformance executable which is created as part of building HighQueue.

You can also see preliminary results from HighSteps by running Steps_Test.

<b><em>Your Performance Mileage Will Vary!</em></b>

