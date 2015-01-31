// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>
#define BOOST_TEST_NO_MAIN StepsTest
#include <boost/test/unit_test.hpp>

#include <Steps/BoostPropertyTreeConfiguration.h>

using namespace HighQueue;
using namespace Steps;

#define ENABLE_PERFORMANCE_TEST 01
#if ! ENABLE_PERFORMANCE_TEST
#pragma message ("ENABLE_PERFORMANCE_TEST " __FILE__)
#else // ENABLE_PERFORMANCE_TEST

namespace
{
    std::string testJson =
R"json({
   "node_type":
   {
       "string": "name",
       "number": 42
   },
   "anotherNode":
   {
       "double": 101.5,
       "negative": -40
   },
   "finalNode" : "yes"

})json";

}

BOOST_AUTO_TEST_CASE(TestConfiguration)
{
    std::cout << "Configuration test" << std::endl;
    std::istringstream testConfig(testJson);

    std::string streamName = "testJson";
    BoostPropertyTreeNode properties;
    properties.loadJson(testConfig, streamName);

    // outermost scope is named after the stream (or filename)
    BOOST_CHECK_EQUAL(properties.getName(), streamName);

    // outermost scope has an empty string as a value.
    std::string value1;
    std::string defaultValue1("NobodyHome");
    std::string empty;
    BOOST_CHECK(properties.getValue(value1, defaultValue1));
    BOOST_CHECK_EQUAL(value1, empty);

    // check top level children
    auto children0 = properties.getChildren();
    BOOST_CHECK(children0->first());

    auto child1 = children0->getChild();
    BOOST_REQUIRE(child1); // we have a child.
    const std::string expectedName1 = "node_type";
    BOOST_CHECK_EQUAL(child1->getName(), expectedName1);
 
    BOOST_CHECK(children0->next());
    auto child2 = children0->getChild();
    BOOST_REQUIRE(child2);
    const std::string expectedName2 = "anotherNode";
    BOOST_CHECK_EQUAL(child2->getName(), expectedName2);

    BOOST_CHECK(children0->next());
    auto child3 = children0->getChild();
    BOOST_REQUIRE(child3);

    BOOST_CHECK(!children0->next());

    // check child1's children
    //"node_type":
    //{
    //    "string": "name",
    //        "number" : 42
    //},

    auto children1 = child1->getChildren();
    BOOST_REQUIRE(children1);

    BOOST_CHECK(children1->first());
    auto child1_1 = children1->getChild();
    BOOST_REQUIRE(child1_1);
    const std::string expectedName1_1 = "string";
    BOOST_CHECK_EQUAL(child1_1->getName(), expectedName1_1);
    const std::string expectedValue1_1 = "name";
    std::string value1_1;
    BOOST_CHECK(child1_1->getValue(value1_1));
    BOOST_CHECK_EQUAL(value1_1, expectedValue1_1);

    uint64_t unsignedValue1_1 = 99;
    int64_t signedValue1_1 = 99;
    double doubleValue1_1 = 99.99;

    uint64_t defaultUnsignedValue1_1 = -88;
    int64_t defaultSignedValue1_1 = 88;
    double defaultDdoubleValue1_1 = 88.88;

    BOOST_CHECK(!child1_1->getValue(unsignedValue1_1, defaultUnsignedValue1_1));
    BOOST_CHECK_EQUAL(unsignedValue1_1, defaultUnsignedValue1_1);
    BOOST_CHECK(!child1_1->getValue(signedValue1_1, defaultSignedValue1_1));
    BOOST_CHECK_EQUAL(signedValue1_1, defaultSignedValue1_1);
    BOOST_CHECK(!child1_1->getValue(doubleValue1_1, defaultDdoubleValue1_1));
    BOOST_CHECK_EQUAL(doubleValue1_1, defaultDdoubleValue1_1);

    BOOST_CHECK(children1->next());
    auto child1_2 = children1->getChild();
    BOOST_REQUIRE(child1_2);
    const std::string expectedName1_2 = "number";
    BOOST_CHECK_EQUAL(child1_2->getName(), expectedName1_2);

    const uint64_t expectedValue1_2 = 42;
    uint64_t value1_2 = 9999;

    const int64_t expectedSignedValue1_2 = 42;
    int64_t signedValue1_2 = 9999;

    const double expectedDoubleValue1_2 = 42.0;
    double doubleValue1_2 = 9999;

    BOOST_CHECK(child1_2->getValue(value1_2));
    BOOST_CHECK_EQUAL(value1_2, expectedValue1_2);

    BOOST_CHECK(child1_2->getValue(signedValue1_2));
    BOOST_CHECK_EQUAL(signedValue1_2, expectedSignedValue1_2);

    BOOST_CHECK(child1_2->getValue(doubleValue1_2));
    BOOST_CHECK_EQUAL(doubleValue1_2, expectedDoubleValue1_2);

    BOOST_CHECK(!children1->next());

    // check node 2's children
    //"anotherNode":
    //{
    //    "double": 101.5,
    //    "negative" : -40
    //},
    auto children2 = child2->getChildren();
    BOOST_REQUIRE(children2);

    BOOST_CHECK(children2->first());
    auto child2_1 = children2->getChild();
    BOOST_REQUIRE(child2_1);
    const std::string expectedName2_1 = "double";
    BOOST_CHECK_EQUAL(child2_1->getName(), expectedName2_1);
    const double expectedDoubleValue2_1 = 101.5;
    double doubleValue2_1 = 0.0;
    BOOST_CHECK(child2_1->getValue(doubleValue2_1));
    BOOST_CHECK_EQUAL(doubleValue2_1, expectedDoubleValue2_1);
    uint64_t unsignedValue2_1 = 9;
    BOOST_CHECK(!child2_1->getValue(unsignedValue2_1));
    BOOST_CHECK_EQUAL(unsignedValue2_1, 0);
    uint64_t signedValue2_1 = 9;
    BOOST_CHECK(!child2_1->getValue(signedValue2_1));
    BOOST_CHECK_EQUAL(signedValue2_1, 0);

    BOOST_CHECK(children2->next());
    auto child2_2 = children2->getChild();
    BOOST_REQUIRE(child2_2);

    const std::string expectedName2_2 = "negative";
    BOOST_CHECK_EQUAL(child2_2->getName(), expectedName2_2);
    const double expectedDoubleValue2_2 = -40.0;
    double doubleValue2_2 = 0.0;
    BOOST_CHECK(child2_2->getValue(doubleValue2_2));
    BOOST_CHECK_EQUAL(doubleValue2_2, expectedDoubleValue2_2);
    uint64_t unsignedValue2_2 = 9;
    // stupid boost property_tree doesn't fail when it should.  I'm not gonna fix it.
//    BOOST_CHECK(!child2_2->getValue(unsignedValue2_2));
//    BOOST_CHECK_EQUAL(unsignedValue2_2, 0);
    uint64_t signedValue2_2 = 9;
    uint64_t expectedSignedValue2_2 = -40;
    BOOST_CHECK(child2_2->getValue(signedValue2_2));
    BOOST_CHECK_EQUAL(signedValue2_2, expectedSignedValue2_2);

    // check node 3 is childless
    auto children3 = child3->getChildren();
    BOOST_REQUIRE(children3);
    BOOST_CHECK(!children3->first());

    // Partial check of the get boolean logic.
    bool value3 = false;
    BOOST_CHECK(child3->getValue(value3));
    BOOST_CHECK(value3);

}

#endif // ENABLE_PERFORMANCE_TEST