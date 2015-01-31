// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>
#define BOOST_TEST_NO_MAIN StepsTest
#include <boost/test/unit_test.hpp>

#include <Steps/Step.h>
#include <Steps/StepFactory.h>
using namespace HighQueue;
using namespace Steps;

#define ENABLE_FACTORY_TEST 01
#if ! ENABLE_FACTORY_TEST
#pragma message ("ENABLE_FACTORY_TEST " __FILE__)
#else // ENABLE_FACTORY_TEST

namespace
{
    class MockStep : public Step
    {
    public:
        MockStep()
        {
            LogTrace("Construct Mock Step @" << (void*)this);
        }
        virtual ~MockStep()
        {
            LogTrace("Destruct Mock Step @" << (void*)this);
        }

    };
}

BOOST_AUTO_TEST_CASE(TestFactory)
{
    std::cout << "TestFactory" << std::endl;

    const std::string StepName = "MockStep";

    auto Stepmaker = [](){ return std::make_shared<MockStep>();};

    StepFactory::registerMaker(StepName, Stepmaker);

    auto madeStep = StepFactory::make(StepName);
    BOOST_CHECK(madeStep);

    auto madeTee = StepFactory::make("tee");
    BOOST_CHECK(madeTee);

    auto madeFail = StepFactory::make("nonexistent_Step");
    BOOST_CHECK(!madeFail);

    StepFactory::list(std::cerr << "Registry: ") << std::endl;
}

#endif // ENABLE_FACTORY_TEST
