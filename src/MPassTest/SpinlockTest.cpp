#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <Common/Spinlock.h>
using namespace MPass;

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
/*
Still to test:

Everything

*/