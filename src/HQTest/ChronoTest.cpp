#include "Common/HighQueuePch.h"

#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <boost/chrono.hpp>
#include <chrono>

BOOST_AUTO_TEST_CASE(testChronoImplementations)
{
    auto stdHigh = std::chrono::nanoseconds(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    auto stdSteady = std::chrono::nanoseconds(std::chrono::steady_clock::now().time_since_epoch()).count();
    auto stdSystem = std::chrono::nanoseconds(std::chrono::system_clock::now().time_since_epoch()).count();

    auto boostHigh   = boost::chrono::nanoseconds(boost::chrono::high_resolution_clock::now().time_since_epoch()).count();
    auto boostSteady = boost::chrono::nanoseconds(boost::chrono::steady_clock::now().time_since_epoch()).count();
    auto boostSystem = boost::chrono::nanoseconds(boost::chrono::system_clock::now().time_since_epoch()).count();
    int64_t resolution = std::numeric_limits<int64_t>::max();
    for(auto sample = 0; sample < 10; ++sample)
    {
        auto prev = std::chrono::nanoseconds(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        auto now = prev;
        while(now == prev)
        {
            now = std::chrono::nanoseconds(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
        auto tick = now - prev;
        if(tick < resolution) resolution = tick;
    }

    std::cout << "Clock  \tstd             \tboost" << std::endl;
    std::cout << "High " << '\t' << stdHigh << '\t' << boostHigh << std::endl;
    std::cout << "Steady " << '\t' << stdSteady << '\t' << boostSteady << std::endl;
    std::cout << "System " << '\t' << stdSystem << '\t' << boostSystem << std::endl;

    std::cout << std::endl << "std::chrono::high_resolution_clock resolution: " << resolution << std::endl;
}
