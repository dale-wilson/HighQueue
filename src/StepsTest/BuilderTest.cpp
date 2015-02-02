// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>
#define BOOST_TEST_NO_MAIN StepsTest
#include <boost/test/unit_test.hpp>

#include <Steps/BoostPropertyTreeConfiguration.h>
#include <Steps/Builder.h>
#include <Steps/Step.h>
#include <Steps/StepFactory.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    std::string testJson1 =
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

    std::string testJson2 =
R"json({
  "pipe": {
    "heartbeat" : {
      "name" : "HeartbeatProducer",
      "milliseconds" : 100
    },
    "small_test_message_consumer" : {
      "name" : "MockMessageConsumer"
    }
  }
}
)json";

    std::string testJson3 =
R"json({
  "pipe": {
    "heartbeat" : {
      "name" : "HeartbeatProducer",
      "milliseconds" : 100
    },
    "send_to_queue" : {
        "name" : "SendToQueue1",
        "queue" : "queue1"
    }
  },
  "pipe": {
    "input_queue" : {
        "name" : "queue1",
        "entry_count" : 100
    },
    "small_test_message_consumer" : {
      "name" : "MockMessageConsumer"
    }
  }
}
)json";

    std::string testJson4 =
R"json({
  "pipe": {
    "heartbeat" : {
      "name" : "HeartbeatProducer",
      "milliseconds" : 100
    },
    "send_to_queue" : {
        "name" : "SendHeartbeatsToQueue1",
        "queue" : "queue1"
    }
  },
  "pipe": {
    "small_test_message_producer" : {
      "name" : "MockMessageProducer",
      "message_count" : 0
    },
    "send_to_queue" : {
        "name" : "SendTestMessagesToQueue1",
        "queue" : "queue1"
    }
  },
  "pipe": {
    "input_queue" : {
        "name" : "queue1",
        "entry_count" : 100
    },
    "small_test_message_consumer" : {
      "name" : "MockMessageConsumer"
    }
  }
}
)json";


    void listLines(const std::string & lines)
    {
        std::istringstream testConfig(lines);
        size_t lineNumber = 0;
        while(!testConfig.eof())
        {
            char line[1000];
            testConfig.getline(line, sizeof(line));
            std::cout << ++lineNumber << ": " << line << std::endl;
        }
    }

    void runBuilderTest(const std::string & testJson)
    {
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
}

#define ENABLE_BUILDER_TEST1 0
#if ENABLE_BUILDER_TEST1

BOOST_AUTO_TEST_CASE(TestBuilder1)
{
    std::cout << "Builder test1" << std::endl;
    listLines(testJson1);
    runBuilderTest(testJson1);
}

#endif // ENABLE_BUILDER_TEST1

#define ENABLE_BUILDER_TEST2 0
#if ENABLE_BUILDER_TEST2

BOOST_AUTO_TEST_CASE(TestBuilder2)
{
    std::cout << "Builder test2" << std::endl;
    listLines(testJson2);
    runBuilderTest(testJson2);
}
#endif // ENABLE_BUILDER_TEST2

#define ENABLE_BUILDER_TEST3 0
#if ENABLE_BUILDER_TEST3

BOOST_AUTO_TEST_CASE(TestBuilder3)
{
    std::cout << "Builder test3" << std::endl;
    listLines(testJson3);
    runBuilderTest(testJson3);
}
#endif // ENABLE_BUILDER_TEST3

#define ENABLE_BUILDER_TEST4 01
#if ENABLE_BUILDER_TEST4

BOOST_AUTO_TEST_CASE(TestBuilder4)
{
    std::cout << "Builder test4" << std::endl;
    listLines(testJson4);
    runBuilderTest(testJson4);
}
#endif // ENABLE_BUILDER_TEST4

