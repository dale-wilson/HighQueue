/// @file Consumer.cpp
#include <Common/HighQueuePch.hpp>
#include "MultiQueueConsumer.hpp"

using namespace HighQueue;

MultiQueueConsumer::MultiQueueConsumer()
: stopping_(false)
, consumerPos_(0)
{
}

MultiQueueConsumer::~MultiQueueConsumer()
{
}

void MultiQueueConsumer::addQueue(ConnectionPtr & connection)
{
    ConsumerPtr consumer = std::make_shared<Consumer>(connection);
    consumer->setName(std::string(connection->getHeader()->name_, sizeof(HQHeader::name_)));
    consumers_.push_back(consumer);
}


void MultiQueueConsumer::stop()
{
    stopping_ = true;
    for(ConsumerVec::iterator it = consumers_.begin(); it != consumers_.end(); ++it)
    {
        (*it)->stop();
    }
}

size_t MultiQueueConsumer::nextConsumer()
{
    consumerPos_ = (consumerPos_ + 1) % consumers_.size();
    return consumerPos_;
}

bool MultiQueueConsumer::tryGetNext(Message & message)
{
    bool found = false;
    size_t start = consumerPos_;
    size_t end = consumerPos_ + consumers_.size();
    for(size_t pos = start; !found && !stopping_ && pos < end; ++pos)
    {
        found = consumers_[pos % consumers_.size()]->tryGetNext(message);
    }
    return found;
}

bool MultiQueueConsumer::getNext(Message & message)
{
    bool found = false;
    while(!found && !stopping_)
    {
        found = tryGetNext(message);
    }
    return found;
}

std::ostream & MultiQueueConsumer::writeStats(std::ostream & out) const
{
    for(ConsumerVec::const_iterator it = consumers_.begin(); it != consumers_.end(); ++it)
    {
        out << (*it)->getName() << ": ";
        (*it)->writeStats(out);
    }
    return out;
}







