# HighQueue
A Queue for High Speed message passing between threads, processes, or other components.

```
  The HighQueue Haiku

  To send messages
  Quickly use clean code and a
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

##Client API
The HighQueue API consists of the public interfaces of four primary classes:   Message, Connection, Consumer, and Producer.  
Three additional classes, CreationParameters, ConsumerWaitStrategy, and MemoryPool, are used to initialize the HighQueue.
###Start with a Connection
A Connection provides access to a HighQueue.  A Connection may be used to create an initialize a new HighQueue, or it may be used to attach to an existing HighQueue (presumably in shared memory.)  Once the HighQueue is created or located, the Connection can be used to initialize Producer and Consumer clients.

####Step-by-step:
  *	Construct a Connection object.  No construction arguments are required.
  *	If the Connection will be used to create a new HighQueue:
    *	Construct a ConsumerWaitStrategy which specifies how Consumers wait for messages
    *	Construct a CreationParameters object containing the ConsumerWaitStrategy and other configuration information such as the number of messages that can be queued, the maximum size of a message, etc.
		* Note that for performance reasons HighQueue does not do any dynamic memory allocation.  Thus it must be configured properly at initialization time to achieve the best performance.
	*   Optionally create a MemoryPool to manage the memory used by Messages.
    *	Call Connection::createLocal() or Connection::createOrOpenShared() passing in the CreationParameters and the optional MemoryPool.
		* If you don't supply a MemoryPool, the Connection will create one based on values from the CreationParameters.  
		* The reasons for explicitly creating a MemoryPool rather than letting the Connection create one will be described below. 
  *	If the Connection will be used only to locate an existing HighQueue in shared memory
    *	Call Connection::openExistingShared();

```C++
    ConsumerWaitStrategy strategy(spinCount, yieldCount); 
    CreationParameters parameters(strategy, entryCount, messageBytes);
    MemoryPoolPtr memoryPool(new MemoryPool(messageBytes, messageCount));
    Connection connection;
	connection->createLocal("HighQueue Name", parameters, memoryPool);   
```
	
This Connection object may now be used to create a Consumer and one or more Producers.

###To Add a Consumer
  *	Construct a Consumer passing the Connection as an argument.
  *	Construct a Message passing the Connection as an argument.
    *	Most consumers will need only one Message which should be created at the time the consumer is being initialized.  This Message will be reused for each message received.
    *	Accept a message from the HighQueue by calling either:
      *	Consumer::tryGetNext(Message &), or
      *	Consumer::getNext(Message &)
    *	If a message is available, these methods are identical. 
    *	If no message is available 
      *	tryGetNext() returns immediately with a false result, whereas 
      *	getNext() will wait for the message to return using the ConsumerWaitStrategy that was specified when the HighQueue was created.
        *	getNext() may also return false if the HighQueue is shutting down.
  *	Use methods on the Message to access the information from the message.
  *	When the consumer no longer needs the contents of a message, it can simply reuse the message for the next tryGetNext() or getNext() call.

```C++
    Consumer consumer(connection);
    Message consumerMessage(connection);
```  
  
###To Add One or More Producers
  *	Construct a Producer passing the Connection as the contruction argument.
  *	Construct a Message passing the Connection as an argument.
    *	Most producers will need only one Message object which should be created at the time the producer is being initialized.  This Message object will be reused for each message being sent.
  *	Populate the Message with the information to be passed to the consumer.
    *	Several methods on the Message make this easy and type-safe.  These methods will be described later.
  *	Publish the message by calling the Producer::publish(Message &); method.
    *	When the publish() method returns, the Message will be empty, ready to be populated with the next message.

```C++
    Producer producer(connection);
    Message producerMessage(connection);
```  

##Populating a Message Before Publishing it
At any particular time a Message object owns a block of memory.  The Producer client should populate
its Message with the data to be sent, then call Producer::publish() to actually send the message.

There are several ways in which the Producer can populate the message depending on what type of
data is being sent and where the Producer gets the data.

###Using Emplace (placement construction) to send an object.

If the data to be delivered is actually an object that the Producer will construct, the most effective
way to populate the Message is to use the emplace method.  This uses a placement new operatation to 
construct the object in place.   [This is similar to the emplace_back() operaton on a std::vector, hence the name emplace]

A typical use might look like:
```C++
	while(! stopping)
   {
        auto myObject = producerMessage.emplace<MyClass>(construction arguments go here);
        myObject.addAdditionalInformationAsNecessary(additional info);
        producer.publish(producerMessage);
   }
```

For local (in-process) HighQueues, there are no restrictions on the type of object that can be passed using this technique.
Shared memory HighQueues impose some restrictions ---todo: document the restrictions [PODS]

Note that if the object to be sent is already present in memory not controlled by the Message, this
technique still works.  Just use the object's copy or move constructor to emplace the data into the 
Message.

###Reading complete messages directly into the Message's buffer.

If the data to be sent comes from a source that can read or compose a data 
into a caller-supplied buffer, you can have the source operate directly into the Message's 
buffer by calling Message::get() and Message::available().

For example:
```C++
    while(! stopping)
    {
        auto buffer = producerMessage.get();
        auto size = producerMessage.available();
        auto bytesRead = DataSource::read(buffer, size);
        producerMessage.setUsed(bytesRead);
        producer.publish(producerMessage);
    }
```

###Sending binary copyable data
If the data to be sent is not in a suitable object for emplacing, but it can be safely copied via 
a binary copy the Message:appendBinaryCopy(pointer, size) operation can be used.  Note any type of 
pointer is acceptable, as long as it points to a contiguous block of memory containing the data to 
be sent.

For example, suppose you have an std::string and you don't wish to copy construct it into the Message
because even a placement new of a string can cause an expensive memory allocation.
You could populate the message like this:

```C++
   while(! stopping)
   {
        const std::string & msg = getSomeDataToSend();
        producerMessage.appendBinaryCopy(msg.data(), msg.size());
        producer.publish(producerMessage);
   }
```

###Sending data from buffer(s) where the message boundaries do not match the buffer boundaries.
i.e. data read from a TCP/IP socket or random-sized messages read from a file.

When the data is split, call appendBinaryCopy twice.

###Et cetera

Other techniques can be used to populate the Message.  
For example fields can be appended to the buffer one-at-a-time in a (relatively) safe manner.
See the append* methods for details.

## Access Data From a Received Message.
The Message used by the Consumer in the tryGetNext() or getNext() method will have access to the
same memory that was used by the Producer to publish the message (note: the Publisher no longer has
access to this memory.)

Several techniques are available to allow the Consumer to safely access this data.

###Accessing data in an application defined object.

The Message::cast&lt;Type&gt;() method will return a reference to an object of the specified type 
residing in the buffer.  This is the most effective way to access incoming data.

Example:
```C++
   while(consumer.getNext(consumerMessage))
   {
        auto myObject = consumerMessage.cast&lt;MyClass&gt>();
        myObject.doSomethingInteresting(); // or
        doSomethingInteresting(myObject);
        consumerMessage.delete&lt;myObject&gt;(); // optional: calls the objects destructor if necessary.
   }
```

###Accessing binary data
```C++
    while(consumer.getNext(consumerMessage))
    {
        auto bytePointer = consumerMessage.get();
        auto size = consumerMessage.getUsed();
        doSomethingInteresting(bytePointer, size);
        // no need to delete binary data, just be sure it is no longer needed
        // before the next call to consumer.getNext();
    }
```

###Et cetera
Again other techniques are available.  See Message::get() for ideas.

##Sharing a pool of memory between multiple HighQueues.
TODO: Explain how to do it and why you might want to.

###API Notes
  *	The producer may take as long as it needs to construct a message directly in the Message. If there are other producers using the same HighQueue they will be unaffected by the time it takes a particular producer to prepare its message for publication.
  *	Clients lose access to information in a message once they use the Message object in a publish() or getNext() call.  In particular pointers and references to the data contained in a message are invalided when the Message object is reused.
  *	When a client is ready to exit, it simply lets the Message and Producer or Consumer objects go out of scope (or otherwise be deleted.)  This cleans up the resources used by the client.
    *	The Connection will still be ready to service additional clients.
  *	The Connection object MUST live longer than all clients and Messages that use it.  
    *	This requirement is not currently enforced.
