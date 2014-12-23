#pragma once

#include <atomic>

namespace MPass
{
    class Spinlock
    {
    public:
        Spinlock();

        class Unguard;
        class Guard
        {
        public:
            Guard(Spinlock & spinlock, bool acquireNow = true);
            ~Guard();
            bool isLocked()const;
            void acquire();
 // todo        bool tryAcquire();
        private:
            friend class Unguard;
            bool release();
        private:
            Spinlock & spinlock_;
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
        std::atomic_flag flag_;
    };

    inline
    Spinlock::Spinlock()
    {
        // Note: due to a defect(..er... severe and stupid limitation) in the C++11 standard 
        // the only way to initialize an atomic_flag is via explicit member initialization
        // (added as part of C++ 11).
        // Alas inline member initialization is not implemented in visual studio (2013).
        // See discussion here:  http://wg21.cmeerw.net/lwg/issue2159
        // and here: http://stackoverflow.com/questions/24437396/stdatomic-flag-as-member-variable
        // Fortunately this seems to work although technically it is undefined behavior.
        flag_.clear();
    }

    inline
    void Spinlock::acquire()
    {
        // todo: consider making the acquisition semantics configurable
        while(flag_.test_and_set(std::memory_order::memory_order_seq_cst))
        {
            // todo: consider an initial spin-only before the first yield
            std::this_thread::yield();
        }
    }

// todo bool Spinlock::tryAcquire();

    inline
    void Spinlock::release()
    {
        flag_.clear();
    }

    inline
    Spinlock::Guard::Guard(Spinlock & spinlock, bool acquireNow)
    : spinlock_(spinlock)
    , owns_(false)
    {
        if(acquireNow)
        {
            acquire();
        }
    }

    inline
    Spinlock::Guard::~Guard()
    {
        if(owns_)
        {
            spinlock_.release();
        }
    }

    inline
    void Spinlock::Guard::acquire()
    {
        spinlock_.acquire();
        owns_ = true;
    }

    //bool Spinlock::Guard::tryAcquire()

    inline
    bool Spinlock::Guard::release()
    {
        bool wasOwned = owns_;
        if(owns_)
        {
            spinlock_.release();
            owns_ = false;
        }
        return wasOwned;
    }

    inline
    bool Spinlock::Guard::isLocked()const
    {
        return owns_;
    }

    inline
    Spinlock::Unguard::Unguard(Guard & guard)
    : guard_(guard)
    , wasLocked_(guard_.release())
    {
    }

    inline
    Spinlock::Unguard::~Unguard()
    {
        if(wasLocked_)
        {
            guard_.acquire();
        }
    }
} // namespace MPass