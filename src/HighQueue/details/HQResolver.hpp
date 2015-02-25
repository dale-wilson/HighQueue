// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/details/HQDefinitions.hpp>

namespace HighQueue
{
    class HighQResolver
    {
    public:
        template <typename T>
        HighQResolver(T * baseAddress)
            : baseAddress_(reinterpret_cast<uint8_t *>(baseAddress))
        {}

        template<typename T>
        T * resolve(Offset offset) const
        {
            return reinterpret_cast<T *>(baseAddress_ + offset);
        }

        template<typename T>
        T * resolve(Offset offset, size_t entrySize, size_t index)
        {
            return reinterpret_cast<T *>(baseAddress_ + offset + entrySize * index);
        }
            
        template<typename T>
        Offset toOffset(const T* target)
        {
            return reinterpret_cast<const uint8_t *>(target) - baseAddress_;
        }
    private:
        uint8_t * baseAddress_;
    };
}
