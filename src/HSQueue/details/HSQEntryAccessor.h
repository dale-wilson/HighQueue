/// @file HSQEntryAccessor.hpp
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HSQueue/details/HSQDefinitions.h>
#include <HSQueue/details/HSQEntry.h>
#include <HSQueue/details/HSQResolver.h>

namespace HSQueue
{
    class HSQEntryAccessor
    {
    public:
        HSQEntryAccessor(HSQResolver & resolver, Offset entryOffset, size_t entryCount);

        HSQEntry & operator[](Position index)const;
    private:
        HSQEntry * entries_;
        size_t entryCount_;
    };

    inline
    HSQEntryAccessor::HSQEntryAccessor(HSQResolver & resolver, Offset entryOffset, size_t entryCount)
        : entries_(resolver.resolve<HSQEntry>(entryOffset))
        , entryCount_(entryCount)
    {
    }

    inline
    HSQEntry & HSQEntryAccessor::operator[](Position index)const
    {
        return entries_[index % entryCount_];
    }

} // HSQueue
