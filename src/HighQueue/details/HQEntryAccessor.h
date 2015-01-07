/// @file HighQEntryAccessor.hpp
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/details/HQDefinitions.h>
#include <HighQueue/details/HQEntry.h>
#include <HighQueue/details/HQResolver.h>

namespace HighQueue
{
    class HighQEntryAccessor
    {
    public:
        HighQEntryAccessor(HighQResolver & resolver, Offset entryOffset, size_t entryCount);

        HighQEntry & operator[](Position index)const;
    private:
        HighQEntry * entries_;
        size_t entryCount_;
    };

    inline
    HighQEntryAccessor::HighQEntryAccessor(HighQResolver & resolver, Offset entryOffset, size_t entryCount)
        : entries_(resolver.resolve<HighQEntry>(entryOffset))
        , entryCount_(entryCount)
    {
    }

    inline
    HighQEntry & HighQEntryAccessor::operator[](Position index)const
    {
        return entries_[index % entryCount_];
    }

} // HighQueue
