/// @file PQEntryAccessor.hpp
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/details/PQDefinitions.h>
#include <ProntoQueue/details/PQEntry.h>
#include <ProntoQueue/details/PQResolver.h>

namespace MPass
{
    namespace ProntoQueue
    {
        class PQEntryAccessor
        {
        public:
            PQEntryAccessor(PQResolver & resolver, Offset entryOffset, size_t entryCount);

            PQEntry & operator[](Position index)const;
        private:
            PQEntry * entries_;
            size_t entryCount_;
        };

        inline
        PQEntryAccessor::PQEntryAccessor(PQResolver & resolver, Offset entryOffset, size_t entryCount)
            : entries_(resolver.resolve<PQEntry>(entryOffset))
            , entryCount_(entryCount)
        {
        }

        inline
        PQEntry & PQEntryAccessor::operator[](Position index)const
        {
            return entries_[index % entryCount_];
        }

    } // ProntoQueue
} // MPass
