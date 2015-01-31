// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StepsSupport/StepPch.h>
#define BOOST_TEST_NO_MAIN StepsTest
#include <boost/test/unit_test.hpp>

#include <StepsSupport/BoostPropertyTreeConfiguration.h>
#include <StepsSupport/Builder.h>
#include <StepsSupport/Step.h>
#include <StepsSupport/StepFactory.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    std::string testJson =
R"json({
  "pipe": {
    "small_test_message_producer" : {
      "name" : "MockMessageProducer",
      "message_count" : 10
    },
    "small_test_message_consumer" : {
      "name" : "MockMessageConsumer"
    }
  }
}
)json";

}


#define ENABLE_BUILDER_TEST 01
#if ! ENABLE_BUILDER_TEST
#pragma message ("ENABLE_BUILDER_TEST " __FILE__)
#else // ENABLE_BUILDER_TEST

BOOST_AUTO_TEST_CASE(TestBuilder)
{
    std::cout << "Builder test" << std::endl;
    {
        std::istringstream testConfig(testJson);
        size_t lineNumber = 0;
        while(!testConfig.eof())
        {
            char line[1000];
            testConfig.getline(line, sizeof(line));
            std::cout << ++lineNumber << ": " << line << std::endl;
        }
    }
    std::istringstream testConfig(testJson);

    std::string streamName = "testJson";
    BoostPropertyTreeNode properties;
    properties.loadJson(testConfig, streamName);

    Builder builder;
    BOOST_REQUIRE(builder.construct(properties));
    builder.start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    builder.stop();
    builder.finish();
}

#endif // ENABLE_BUILDER_TEST