// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include <Steps/BoostPropertyTreeConfiguration.hpp>
#include <Steps/Builder.hpp>
#include <Steps/Step.hpp>
#include <Steps/StepFactory.hpp>
using namespace HighQueue;
using namespace Steps;

namespace
{
    void listLines(std::istream & file)
    {
        size_t lineNumber = 0;
        while(file.good())
        {
            char line[1000];
            file.getline(line, sizeof(line));
            if(file.good())
            {
                std::cout << ++lineNumber << ": " << line << std::endl;
            }
        }
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
    }
    if(configFile_.good())
    {
        ok = true;
    }
    else
    {
        std::cerr << "Can't open " << configFileName_ << std::endl;
    }
    return ok;
}

void BuilderApp::run()
{
    listLines(configFile_);
    configFile_.close();
    configFile_.open(configFileName_);

    BoostPropertyTreeNode properties;
    properties.loadJson(configFile_, configFileName_);

    Builder builder;
    if(builder.construct(properties))
    {
        std::cout << "Any key + Enter:";
        builder.start();
        char anyKey = 0;
        std::cin >> anyKey;
//        std::this_thread::sleep_for(std::chrono::seconds(5));
        builder.stop();
        builder.finish();
    }
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
