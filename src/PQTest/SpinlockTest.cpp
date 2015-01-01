#include "Common/ProntoQueuePch.h"
#define BOOST_TEST_NO_MAIN ProntoQueueTest
#include <boost/test/unit_test.hpp>

#include <Common/Spinlock.h>
using namespace ProntoQueue;

#define DISABLE_testSpinlockx
#ifdef DISABLE_testSpinlock
#pragma message("DISABLE_testSpinlock")
#else // DISABLE_testSpinlock
BOOST_AUTO_TEST_CASE(testSpinlock)
{
    Spinlock lock;
    {
        Spinlock::Guard guard(lock);
        {
            BOOST_CHECK(guard.isLocked());
            {
                Spinlock::Unguard unguard(guard);
                BOOST_CHECK(!guard.isLocked());
            }
            BOOST_CHECK(guard.isLocked());
        }
    }
}
#endif // DISABLE_testSpinlock
/*
Still to test:

Everything

*/