// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>
#define BOOST_TEST_NO_MAIN StagesTest
#include <boost/test/unit_test.hpp>

#include <StagesSupport/BoostPropertyTreeConfiguration.h>
#include <StagesSupport/Builder.h>

using namespace HighQueue;
using namespace Stages;

#define ENABLE_PERFORMANCE_TEST 01
#if ! ENABLE_PERFORMANCE_TEST
#pragma message ("ENABLE_PERFORMANCE_TEST " __FILE__)
#else // ENABLE_PERFORMANCE_TEST

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

//bool build(std::string &configFileName);
//bool build(std::istream & configFile, const std::string & name);
//
//bool Builder::build(std::string &configFileName)
//{
//    BoostPropertyTreeNode config;
//    config.loadJson(configFileName);
//    return load(config);
//}
//
//bool Builder::build(std::istream & configFile, const std::string & name)
//{
//    BoostPropertyTreeNode config;
//    config.loadJson(configFile, name);
//    return load(config);
//}


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

#endif // ENABLE_PERFORMANCE_TEST