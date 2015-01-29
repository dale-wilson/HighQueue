// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>
#define BOOST_TEST_NO_MAIN StagesTest
#include <boost/test/unit_test.hpp>

#include <StagesSupport/BoostPropertyTreeConfiguration.h>
#include <StagesSupport/Builder.h>
#include <StagesSupport/Stage.h>
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    std::string testJson =
R"json({
   "memory_pool":
   {
       "name": "pool1",
       "message_size": 42,
       "message_count" : 1000
   },
   "asio":
   {
       "name" : "asio1",
       "thread_count" : 1
   },
   "queue":
   {
        "name" : "queue1",
        "memory_pool" : "pool1",
        "common_wait_strategy" : 
         {
             "spin_count" : 0,
             "yield_count" : "forever"
         },
         "discard_messages_if_no_consumer" : false,
         "entry_count" : 100
   }
})json";

}


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

#endif // ENABLE_BUILDER_TEST

#define ENABLE_BUILDER_TEST 01
#if ! ENABLE_BUILDER_TEST
#pragma message ("ENABLE_BUILDER_TEST " __FILE__)
#else // ENABLE_BUILDER_TEST


BOOST_AUTO_TEST_CASE(TestBuilder)
{
    std::cout << "Builder test" << std::endl;
    std::istringstream testConfig(testJson);

    std::string streamName = "testJson";
    auto properties = std::make_shared<BoostPropertyTreeNode>();
    properties->loadJson(testConfig, streamName);

    Builder builder;
    builder.construct(properties);

}

#endif // ENABLE_BUILDER_TEST