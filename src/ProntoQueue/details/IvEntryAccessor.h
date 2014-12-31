/// @file IvEntryAccessor.hpp
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/IvDefinitions.h>
#include <ProntoQueue/details/IvEntry.h>
#include <ProntoQueue/details/IvResolver.h>

namespace MPass
{
    namespace ProntoQueue
    {
        class IvEntryAccessor
        {
        public:
            IvEntryAccessor(IvResolver & resolver, Offset entryOffset, size_t entryCount);

            IvEntry & operator[](Position index)const;
        private:
            IvEntry * entries_;
            size_t entryCount_;
        };

        inline
        IvEntryAccessor::IvEntryAccessor(IvResolver & resolver, Offset entryOffset, size_t entryCount)
            : entries_(resolver.resolve<IvEntry>(entryOffset))
            , entryCount_(entryCount)
        {
        }

        inline
        IvEntry & IvEntryAccessor::operator[](Position index)const
        {
            return entries_[index % entryCount_];
        }

    } // ProntoQueue
} // MPass
