#include <Common/HighQueuePch.hpp>
#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <Common/SpinLock.hpp>
using namespace HighQueue;

#define DISABLE_testSpinLockx
#ifdef DISABLE_testSpinLock
#pragma message("DISABLE_testSpinLock")
#else // DISABLE_testSpinLock
BOOST_AUTO_TEST_CASE(testSpinLock)
{
    SpinLock lock;
    {
        SpinLock::Guard guard(lock);
        {
            BOOST_CHECK(guard.isLocked());
            {
                SpinLock::Unguard unguard(guard);
                BOOST_CHECK(!guard.isLocked());
            }
            BOOST_CHECK(guard.isLocked());
        }
    }
}
#endif // DISABLE_testSpinLock
/*
Still to test:

Everything

*/