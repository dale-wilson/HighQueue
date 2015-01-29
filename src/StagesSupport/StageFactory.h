// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>
#include <StagesSupport/StageFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export StageFactory
        {
            StageFactory() = delete;
            ~StageFactory() = delete;
        public:
            typedef std::function<StagePtr ()> Maker;

            static void registerMaker(const std::string & name, const Maker & maker);
            static StagePtr make(const std::string & name);
            static std::ostream & list(std::ostream & out);
        };

        template <typename StageType>
        struct Registrar
        {
            Registrar(const std::string & name)
            {
                StageFactory::registerMaker(name, [name]()
                        {
                            LogTrace("StageFactory Registrar constructing " << name);
                            return std::make_shared<StageType>();
                        }
                    );
            }
        };
   }  
}