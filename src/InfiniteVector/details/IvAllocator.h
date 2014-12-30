/// @file IvAllocator.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "IvDefinitions.h"

namespace MPass
{
	namespace InfiniteVector
	{
        /// @brief A helper class to assist in memory allocation in an infinite vector.
        /// Works in offsets
		class IvAllocator
		{
		public:
            /// @brief Construct 
            /// @param available how many bytes do we have from which to allocate?
            /// @param initialPosition where should allocation begin?
            explicit IvAllocator(
                size_t available,
                Offset initialPosition = 0)
            : available_(available)
            , position_(initialPosition)
            {
            }

            /// @brief Align the current position, return the offset to it, and increment the position by size.
            /// @param size is the sizeof the object that will use the memory.
            /// @param aligment is the allignment needed by the object
            /// @returns the offset to the first byte of the object.
            Offset allocate(size_t size, size_t alignment)
            {
                Offset location = align(position_, alignment);
                position_ += Offset(size);
                return location;
            }

            /// @brief How much is left after aligning the current position?
            size_t available(size_t alignment)const
            {
                return available_ - align(position_, alignment);
            }

            /// @brief A helper method to align an integer.
            template<typename IntegerType>
            static IntegerType align(IntegerType offset, size_t alignment = CacheLineSize)
            {
                return (alignment <=  1) ? offset : Offset(((offset + alignment - 1) / alignment) * alignment);
            }

            /// @brief A helper method to align a pointer.
            template<typename T>
            static T * align(T * pointer, size_t byteAlignment = CacheLineSize)
            {
                return reinterpret_cast<T*>(align(intptr_t(pointer), byteAlignment));
            }

        private:
            size_t available_;
            Offset position_;
		};
	}
}