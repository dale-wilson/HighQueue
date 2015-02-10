// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

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
      "name" : "MockMessageProducer_A",
      "message_count" : 100000,
      "producer_number" : 0
    },
    "send_to_queue" : {
        "name" : "SendTestMessagesToQueue1_A",
        "queue" : "queue1"
    }
  },
  "pipe": {
    "small_test_message_producer" : {
      "name" : "MockMessageProducer_B",
      "message_count" : 400000,
      "producer_number" : 0
    },
    "send_to_queue" : {
        "name" : "SendTestMessagesToQueue1_B",
        "queue" : "queue1"
    }
  },
  "pipe": {
    "input_queue" : {
        "name" : "queue1",
        "entry_count" : 100
    },
    "shuffler" : {
        "name" : "shuffler",
        "look_ahead" : 100
    },
    "ordered_merge" : {
        "name" : "merge",
        "look_ahead" : 1000
    },
    "small_test_message_consumer" : {
      "name" : "MockMessageConsumer"
    }
  }
}
)json";

    std::string testJson5 =
R"json({
  "pipe": {
    "small_test_message_producer" : {
      "name" : "MockMessageProducer",
      "message_count" : 10
    },
    "binary_copy" : {
        "name" : "BinaryCopy"
    },
    "small_test_message_copy" : {
        "name" : "ConstructCopy"
    },
    "forward_pass_thru" : {
        "name" : "ForwardPassThru"
    },
    "tee" : {
        "name" : "tee",
        "output" : "null"
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
        builder.construct(properties);
        builder.start();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        builder.stop();
        builder.finish();
    }
}

class BuilderApp
{
public:
    BuilderApp();
    ~BuilderApp();

    void usage(const char * appName);
    bool parseArgs(int argc, char *argv[]);
    bool validate();
    void run();
private:
    std::string configFileName_;
    std::ifstream configFile_;

};

BuilderApp::BuilderApp()
{
}

BuilderApp::~BuilderApp()
{
}

void BuilderApp::usage(const char * appName)
{
    if(!appName)
    {
        appName = "Builder";
    }
    std::cerr << "Usage: " << appName << " [json_config_file] " << std::endl;
}

bool BuilderApp::parseArgs(int argc, char *argv[])
{
    bool ok = false;
    if(argc > 1)
    {
        configFileName_ = argv[1];
        ok = true;
    }
    else
    {
        std::cerr << "Missing required argument" << std::endl;
    }
    return ok;
}

bool BuilderApp::validate()
{
    bool ok = false;
    if(!configFileName_.empty())
    {
        configFile_.open(configFileName_);
        if(configFile_.good())
        {
            ok = true;
        }
        else
        {
            std::cerr << "Can't open " << configFileName_ << std::endl;
        }
    }
    return configFile_.good();
}

void BuilderApp::run()
{

    std::stringstream file;
    while(!configFile_.eof())
    {
        char line[1000];
        configFile_.getline(line, sizeof(line));
        file << line;
        file << '\n';
    }
    std::string cfg = file.str();
    listLines(cfg);
    runBuilderTest(cfg);
}

int main(int argc, char * argv[])
{
    int ok = -1;
    try
    {
        BuilderApp app;
        if(app.parseArgs(argc, argv))
        {
            if(app.validate())
            {
                app.run();
                ok = 0;
            }
        }
        if(ok != 0)
        {
            app.usage(argv[0]);
        }
    }
    catch(const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    return ok;
}
