# HighQueue
A Queue for High Speed message passing between threads, processes, or other components.

<pre>
  The HighQueue Haiku

  To send messages
  So quickly you must use a
  Good algorithm.
</pre>

##Introduction
A High Performance Queue (HighQueue) is a mechanism for message passing between components of a system.  It is most often used to pass messages between two or more threads in the same process, but it can also be used for intra-process communication by putting the HighQueue in shared memory. 
###Clients
HighQueue clients can be either a Publisher or a Consumer.
  *	Producers create and publish data into the HighQueue.   
  * Consumers accept and process data from the HighQueue.
  
Each HighQueue instance supports multiple Producers, but only a single Consumer.
##Client API
The HighQueue API consists of the public interfaces of four primary classes:   Message, Connection, Consumer, and Producer.  Two additional classes, CreationParameters and ConsumerWaitStrategy are used to initialize the HighQueue.
###Start with a Connection
A Connection provides access to a HighQueue.  A Connection may be used to create an initialize a new HighQueue, or it may be used to attach to an existing HighQueue (presumably in shared memory.)  Once the HighQueue is created or located, the Connection can be used to initialize Producer and Consumer clients.

####Step-by-step:
  *	Construct a Connection object.  No construction arguments are required.
  *	If the Connection will be used to create a new HighQueue:
    *	Construct a ConsumerWaitStrategy which specifies how Consumers wait for messages
    *	Construct a CreationParameters object containing the ConsumerWaitStrategy and other configuration information such as the number of messages that can be queued, the maximum size of a message, etc.
      *	Note that for performance reasons HighQueue does not do any dynamic memory allocation.  Thus it must be configured properly at initialization time to achieve the best performance.
    *	Call Connection::createLocal() or Connection::createOrOpenShared().
  *	If the Connection will be used only to locate an existing HighQueue in shared memory
    *	Call Connection::openExistingShared();

This Connection object may now be used to create a Consumer and one or more Producers.

###To Add a Consumer
  *	Construct a Consumer passing the Connection as an argument.
  *	Construct a Message with no construction arguments.
  *	Use the Connection::allocate(Message) to initialize the Message.
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

###To Add One or More Producers
  *	Construct a Producer passing the Connection as the contruction argument.
  *	Construct a Message with no construction arguments.
    *	Most producers will need only one Message object which should be created at the time the producer is being initialized.  This Message object will be reused for each message being sent.
  *	Call Connection::allocate(Message &) to initialize the message.
  *	Populate the Message with the information to be passed to the consumer.
    *	Several methods on the Message make this easy and type-safe.  These methods will be described later.
  *	Publish the message by calling the Producer::publish(Message &); method.
    *	When the publish() method returns, the Message will be empty, ready to be populated with the next message.

###API Notes
  *	The producer may take as long as it needs to construct a message directly in the Message.  If there are other producers using the same HighQueue they will be unaffected by the time it takes a particular producer to prepare its message for publication. (See the example below.)
  *	Clients lose access to information in a message once they use the Message object in a publish() or getNext() call.  In particular pointers to the data contained in a message are invalided when the Message object is reused.
  *	When a client is ready to exit, it simply lets the Message and Producer or Consumer objects go out of scope (or otherwise be deleted.)  This cleans up the resources used by the client.
    *	The Connection will still be ready to service additional clients.
  *	The Connection object MUST live longer than all clients and Messages that use it.  
    *	This requirement is not currently enforced.

##Populating a Message Before Publishing it
To be done

## Access Data From a Received Message.
To Be done

