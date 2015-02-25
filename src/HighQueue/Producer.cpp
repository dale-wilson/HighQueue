#include <Common/HighQueuePch.hpp>
#include "Producer.hpp"
#include <HighQueue/details/HQReservePosition.hpp>

using namespace HighQueue;

namespace {
    const bool useSpinLock = true;
}

Producer::Producer(ConnectionPtr & connection)
: connection_(connection)
, solo_(connection_->canSolo())
, stopping_(false)
, header_(connection_->getHeader())
, entryCount_(header_->entryCount_)
, waitStrategy_(header_->producerWaitStrategy_)
, consumerUsesMutex_(header_->consumerWaitStrategy_.mutexUsed_)
, discardMessagesIfNoConsumer_(header_->discardMessagesIfNoConsumer_)
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, reserveStructure_(*resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_))
, reservePosition_(reserveStructure_.reservePosition_)
, reserveSoloPosition_(reinterpret_cast<volatile Position &>(reservePosition_))
, reserveSpinLock_(const_cast<SpinLock &>(reserveStructure_.reserveSpinLock_))
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, publishable_(0)
, statFulls_(0)
, statSkips_(0)
, statPublishWaits_(0)
, statPublishInLine_(0)
, statPublishes_(0)
, statSpins_(0)
, statYields_(0)
, statSleeps_(0)
, statWaits_(0)
{
    ++header_->producersPresent_;
}

Producer::~Producer()
{
    --header_->producersPresent_;
}

inline bool Producer::unreserve(Position reserve)
{
    if(solo_)
    {
        reserveSoloPosition_ = reserve;
        publishPosition_ = reserve;
        return true;
    }
    else
    {
        if(reserve <= reservePosition_)
        {
            Position expected = reserve + 1;
            return reservePosition_.compare_exchange_strong(expected, reserve);
        }
        return true;
    }
}

inline
bool Producer::canPublish(Position position)
{
    if(publishable_ > position)
    {
        return true;
    }
    publishable_ = readPosition_ + entryCount_;
    if(publishable_ > position)
    {
        return true;
    }
    ++statFulls_;
    if(discardMessagesIfNoConsumer_ && !header_->consumerPresent_)
    {
        ++statDiscards_;
        readPosition_ = publishPosition_;
        publishable_ = readPosition_ + entryCount_;
        return true;
    }
    return false;
}


inline
void Producer::waitToPublish(Position position)
{
    size_t remainingSpins = waitStrategy_.spinCount_;
    size_t remainingYields = waitStrategy_.yieldCount_;
    size_t remainingSleeps = waitStrategy_.sleepCount_;
    while(!canPublish(position))
    {
        if(stopping_ && unreserve(position))
        {
            return;
        }

        if(remainingSpins > 0)
        {
            spinDelay();
            ++statSpins_;
            if(remainingSpins != WaitStrategy::FOREVER)
            {
                --remainingSpins;
            }
            std::atomic_thread_fence(std::memory_order::memory_order_consume);
        }
        else if(remainingYields > 0)
        {
            ++statYields_;
            if(remainingYields != WaitStrategy::FOREVER)
            {
                --remainingYields;
            }
            std::this_thread::yield();
        }
        else if(remainingSleeps > 0)
        {
            ++statSleeps_;
            if(remainingSleeps != WaitStrategy::FOREVER)
            {
                --remainingSleeps;
            }
            std::this_thread::sleep_for(waitStrategy_.sleepPeriod_);
        }
        else
        {
            ++statWaits_;
            std::unique_lock<std::mutex> guard(header_->waitMutex_);
            publishable_ = readPosition_ + entryCount_;
            if(publishable_ <= position)
            {
                header_->producerWaiting_ = true;
                if(header_->producerWaitConditionVariable_.wait_for(guard, waitStrategy_.mutexWaitTimeout_)
                    == std::cv_status::timeout)
                {
                    publishable_ = readPosition_ + entryCount_;
                    if(publishable_ <= position)
                    {
                        // todo: define a better exception
                        throw std::runtime_error("Producer wait timeout.");
                    }
                }
            }
        }
        publishable_ = readPosition_ + entryCount_;
    }
}

inline
bool Producer::publish(Position reserved, Message & message)
{
    HighQEntry & entry = entryAccessor_[reserved];
    if(entry.status_ != HighQEntry::Status::SKIP)
    {
        message.moveTo(entry.message_);
        entry.status_ = HighQEntry::Status::OK;
        return true;
    }
    ++statSkips_;
    return false;
}

void Producer::notifyConsumer()
{
    if(!consumerUsesMutex_)
    {
        std::atomic_thread_fence(std::memory_order::memory_order_release);
        return;
    }

    std::unique_lock<std::mutex> guard(header_->waitMutex_);
    if(header_->consumerWaiting_)
    {
        header_->consumerWaiting_ = false;
        header_->consumerWaitConditionVariable_.notify_all();
    }
}

void Producer::publish(Message & message)
{
    if(solo_)
    {
        bool published = false;
        while(!published && !stopping_)
        {
            auto position = publishPosition_;
            if(canPublish(position))
            {
                published = publish(position, message);
                publishPosition_ = position + 1;
            }
            else
            {
                waitToPublish(position);
            }
        }
        notifyConsumer();
        return;
    }

    size_t remainingSpins = waitStrategy_.spinCount_;
    size_t remainingYields = waitStrategy_.yieldCount_;
    size_t remainingSleeps = waitStrategy_.sleepCount_;
    bool mutexTimedOut = false;
    bool published = false;
    SpinLock::Guard guard(reserveSpinLock_);
    while(!published)
    {
        auto position = publishPosition_;
        if(canPublish(position))
        {
            published = publish(position, message);
            publishPosition_ = position + 1;
        }
        else
        {
            SpinLock::Unguard unguard(guard);
            if(stopping_ )
            {
                return;
            }

            if(remainingSpins > 0)
            {
                spinDelay();
                ++statSpins_;
                if(remainingSpins != WaitStrategy::FOREVER)
                {
                    --remainingSpins;
                }
                std::atomic_thread_fence(std::memory_order::memory_order_consume);
            }
            else if(remainingYields > 0)
            {
                ++statYields_;
                if(remainingYields != WaitStrategy::FOREVER)
                {
                    --remainingYields;
                }
                std::this_thread::yield();
            }
            else if(remainingSleeps > 0)
            {
                ++statSleeps_;
                if(remainingSleeps != WaitStrategy::FOREVER)
                {
                    --remainingSleeps;
                }
                std::this_thread::sleep_for(waitStrategy_.sleepPeriod_);
            }
            else if(!mutexTimedOut)
            {
                ++statWaits_;
                std::unique_lock<std::mutex> mutexGuard(header_->waitMutex_);
                // This position was acquired without the spinlock. 
                // Check it, but don't use it to publish!
                position = publishPosition_;
                if(!canPublish(position))
                {
                    header_->producerWaiting_ = true;
                    mutexTimedOut = (header_->producerWaitConditionVariable_.wait_for(mutexGuard, waitStrategy_.mutexWaitTimeout_)
                        == std::cv_status::timeout);
                }
            }
            else
            {
                // todo: define a better exception
                throw std::runtime_error("Producer wait timeout.");
            }
        }
    }
    notifyConsumer();
}

void Producer::publish2(Message & message)
{
    if(solo_)
    {
        bool published = false;
        while(!published && !stopping_)
        {
            Position reserved = publishPosition_;
            waitToPublish(reserved);
            published = publish(reserved, message);
            publishPosition_ = reserved + 1;
            reserveSoloPosition_ = reserved + 1;
            notifyConsumer();
        }
        return;
    }

    bool published = false;
    while(!published)
    {
        SpinLock::Guard guard;
        if(useSpinLock)
        {
            guard = SpinLock::Guard(reserveSpinLock_);
        }
        auto reserved = reservePosition_++;
        waitToPublish(reserved);
        published = publish(reserved, message);

        int64_t pending = reserved - publishPosition_;
        while(pending > 0)
        {
            /// todo simplify this now that we aren't using it!
            if(stopping_ && unreserve(reserved))
            {
                return;
            }

            ++statPublishWaits_;
            if(pending > 1)
            {
                ++statPublishInLine_;
                std::this_thread::yield();
            }
            else
            {
                spinDelay();
                std::atomic_thread_fence(std::memory_order::memory_order_acquire);
            }
            pending = reserved - publishPosition_;
        }

        // The following will *always* be true unless something is broken
        if(pending == 0)
        {
            ++publishPosition_;
            ++statPublishes_;
            notifyConsumer();
        }
    }
}

void Producer::stop()
{
    stopping_ = true;
    std::unique_lock<std::mutex> guard(header_->waitMutex_);
    header_->producerWaitConditionVariable_.notify_all();
}

std::ostream & Producer::writeStats(std::ostream & out) const
{
    return out << "Published " << statPublishes_
               << " Full: " << statFulls_
               << " Skip: " << statSkips_
               << " WaitOtherPublishers: " << statPublishWaits_
               << "/" << statPublishInLine_
               << " Spin: " << statSpins_ 
               << " Yield: " << statYields_ 
               << " Sleep: " << statSleeps_ 
               << " Wait: " << statWaits_
               << " OK: " << statPublishes_
               << " Solo: " << (solo_ ? "Yes" : "No")
               << std::endl;

}


