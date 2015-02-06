// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <atomic>
#define _USE_CRITICAL_SECTION 1
namespace HighQueue
{
    class SpinLock
    {
    public:
        SpinLock();

        class Unguard;
        class Guard
        {
        public:
            /// @brief RAII constructor.
            Guard(SpinLock & spinlock, bool acquireNow = true);
            /// @brief Move constructor allows delayed attachment to spin lock
            Guard(Guard && spinlock);   

            /// @Construct an unattached guard
            Guard();

            /// @brief Copy would be a disaster
            Guard(const Guard &) = delete;
            /// @brief RAII destructor
            ~Guard();

            /// @brief assignment is move
            Guard & operator =(Guard && rhs);

            bool isLocked()const;
            void acquire();
 // todo        bool tryAcquire();
        private:
            friend class Unguard;
            bool release();
        private:
            SpinLock * spinlock_;
            bool owns_;
        };

        class Unguard
        {
        public:
            Unguard(Guard & guard);
            ~Unguard();
        private:
            Guard & guard_;
            bool wasLocked_;
        };

    private:
        friend class Guard;
        void acquire();
// todo        bool tryAcquire();
        void release();

    private:
#if defined(_WIN32) && _USE_CRITICAL_SECTION
        CRITICAL_SECTION criticalSection_;
#else // defined(_WIN32) && _USE_CRITICAL_SECTION
        std::atomic_flag flag_;
#endif // defined(_WIN32) && _USE_CRITICAL_SECTION
    };

    inline
    SpinLock::SpinLock()
    {
#if defined(_WIN32) && _USE_CRITICAL_SECTION
        InitializeCriticalSection(&criticalSection_);
#else // defined(_WIN32) && _USE_CRITICAL_SECTION
        // Note: due to a defect(..er... severe and stupid limitation) in the C++11 standard 
        // the only way to initialize an atomic_flag is via explicit member initialization
        // (added as part of C++ 11).
        // Alas inline member initialization is not implemented in visual studio (2013).
        // See discussion here:  http://wg21.cmeerw.net/lwg/issue2159
        // and here: http://stackoverflow.com/questions/24437396/stdatomic-flag-as-member-variable
        // Fortunately this seems to work although technically it is undefined behavior.
        flag_.clear();
#endif // defined(_WIN32) && _USE_CRITICAL_SECTION
    }

    inline
    void SpinLock::acquire()
    {
#if defined(_WIN32) && _USE_CRITICAL_SECTION
        EnterCriticalSection(&criticalSection_);
#else defined(_WIN32) && _USE_CRITICAL_SECTION
        while(flag_.test_and_set(std::memory_order::memory_order_seq_cst))
        {
            spinDelay();
        }
#endif // defined(_WIN32) && _USE_CRITICAL_SECTION
    }

// todo bool SpinLock::tryAcquire();

    inline
    void SpinLock::release()
    {
#if defined(_WIN32) && _USE_CRITICAL_SECTION
        LeaveCriticalSection(&criticalSection_);
#else // defined(_WIN32) && _USE_CRITICAL_SECTION
        flag_.clear();
#endif // defined(_WIN32) && _USE_CRITICAL_SECTION
    }

    inline
    SpinLock::Guard::Guard(SpinLock & spinlock, bool acquireNow)
    : spinlock_(&spinlock)
    , owns_(false)
    {
        if(acquireNow)
        {
            acquire();
        }
    }

    inline
    SpinLock::Guard::Guard(SpinLock::Guard && rhs)
    : spinlock_(rhs.spinlock_)
    , owns_(rhs.owns_)
    {
        rhs.spinlock_ = 0;
        rhs.spinlock_ = false;
    }

    inline
    SpinLock::Guard::Guard()
    : spinlock_(0)
    , owns_(false)
    {
    }


    inline
    SpinLock::Guard & SpinLock::Guard::operator= (SpinLock::Guard && rhs)
    {
        release();
        std::swap(spinlock_, rhs.spinlock_);
        std::swap(owns_, rhs.owns_);
        return *this;
    }

    inline
    SpinLock::Guard::~Guard()
    {
        if(owns_ && spinlock_)
        {
            spinlock_->release();
        }
    }

    inline
    void SpinLock::Guard::acquire()
    {
        if(spinlock_)
        { 
            spinlock_->acquire();
            owns_ = true;
        }
    }

    //bool SpinLock::Guard::tryAcquire()

    inline
    bool SpinLock::Guard::release()
    {
        bool wasOwned = owns_;
        if(owns_ && spinlock_)
        {
            spinlock_->release();
            owns_ = false;
        }
        return wasOwned;
    }

    inline
    bool SpinLock::Guard::isLocked()const
    {
        return owns_;
    }

    inline
    SpinLock::Unguard::Unguard(Guard & guard)
    : guard_(guard)
    , wasLocked_(guard_.release())
    {
    }

    inline
    SpinLock::Unguard::~Unguard()
    {
        if(wasLocked_)
        {
            guard_.acquire();
        }
    }

} // namespace HighQueue
