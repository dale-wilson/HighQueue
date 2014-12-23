/// @file IvEntryAccessor.hpp
#pragma once

#include <InfiniteVector/IvDefinitions.h>
#include <InfiniteVector/IvEntry.h>
#include <InfiniteVector/IvResolver.h>

namespace MPass
{
    namespace InfiniteVector
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

    } // InfiniteVector
} // MPass
