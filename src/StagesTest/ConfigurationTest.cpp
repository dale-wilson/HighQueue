// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>
#define BOOST_TEST_NO_MAIN StagesTest
#include <boost/test/unit_test.hpp>

#include <StageCommon/BoostPropertyTreeConfiguration.h>

using namespace HighQueue;
using namespace Stages;

#define ENABLE_PERFORMANCE_TEST 01
#if ! ENABLE_PERFORMANCE_TEST
#pragma message ("ENABLE_PERFORMANCE_TEST " __FILE__)
#else // ENABLE_PERFORMANCE_TEST

BOOST_AUTO_TEST_CASE(TestConfiguration)
{
    std::cout << "Configuration test" << std::endl;
    BOOST_CHECK(true);
}

#endif // ENABLE_PERFORMANCE_TEST