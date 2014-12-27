/// @file IvAllocator.h
#pragma once
#include "IvDefinitions.h"
#include "IvAllocator_fwd.h"
namespace MPass
{
	namespace InfiniteVector
	{
		class IvAllocator
		{
		public:
            explicit IvAllocator(
                size_t available,
                Offset initialPosition = 0)
            : available_(available)
            , position_(initialPosition)
            {
            }

            Offset allocate(size_t size, size_t alignment)
            {
                Offset location = align(position_, alignment);
                position_ += Offset(size);
                return location;
            }

            size_t available(size_t alignment)const
            {
                return available_ - position_;
            }

            template<typename IntegerType>
            static IntegerType align(IntegerType offset, size_t alignment = CacheLineSize)
            {
                return (alignment <=  1) ? offset : Offset(((offset + alignment - 1) / alignment) * alignment);
            }

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