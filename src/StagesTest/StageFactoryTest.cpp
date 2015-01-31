// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>
#define BOOST_TEST_NO_MAIN StagesTest
#include <boost/test/unit_test.hpp>

#include <StagesSupport/Stage.h>
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

#define ENABLE_FACTORY_TEST 01
#if ! ENABLE_FACTORY_TEST
#pragma message ("ENABLE_FACTORY_TEST " __FILE__)
#else // ENABLE_FACTORY_TEST

namespace
{
    class MockStage : public Stage
    {
    public:
        MockStage()
        {
            LogTrace("Construct Mock Stage @" << (void*)this);
        }
        virtual ~MockStage()
        {
            LogTrace("Destruct Mock Stage @" << (void*)this);
        }

    };
}

BOOST_AUTO_TEST_CASE(TestFactory)
{
    std::cout << "TestFactory" << std::endl;

    const std::string stageName = "MockStage";

    auto stagemaker = [](){ return std::make_shared<MockStage>();};

    StageFactory::registerMaker(stageName, stagemaker);

    auto madeStage = StageFactory::make(stageName);
    BOOST_CHECK(madeStage);

    auto madeTee = StageFactory::make("tee");
    BOOST_CHECK(madeTee);

    auto madeFail = StageFactory::make("nonexistent_stage");
    BOOST_CHECK(!madeFail);

    StageFactory::list(std::cerr << "Registry: ") << std::endl;
}

#endif // ENABLE_FACTORY_TEST
