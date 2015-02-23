// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Step_Export.h>
#include <Steps/StepFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export StepFactory
        {
            StepFactory() = delete;
            ~StepFactory() = delete;
        public:
            template <typename StepType>
            struct Registrar
            {
                Registrar(const std::string & name, const std::string & description)
                {
                    StepFactory::registerMaker(name, description, 
                        [name]()
                        {
                            LogTrace("StepFactory Registrar constructing " << name);
                            return std::make_shared<StepType>();
                        }
                    );
                }
            };

            typedef std::function<StepPtr()> Maker;

            static void registerMaker(const std::string & name, const std::string & description, const Maker & maker);
            static StepPtr make(const std::string & name);
            static std::ostream & list(std::ostream & out);
        };

   }  
}