// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Buffers/MemoryBlockPoolFwd.h>
namespace MPass
{
    namespace Buffers
    {
        /// @brief A handle for a block of memory
        class Buffer
        {
        public:
            enum Type
            {
                Normal,
                Orphan,
                Borrowed,
                Invalid
            };
            const static size_t NO_POOL = ~size_t(0);

            /// @brief construct an empty (Invalid) buffer.
            Buffer();

            ~Buffer();

            /// @brief return a pointer to the block of memory cast to the requested type.  
            ///
            /// This should be used when the buffer already contains an object of the appropriate type.
            /// or when the caller plans to construct/initialize the contents of the buffer.
            /// The memory is not changed by this call, 
            ///          
            /// @tparam T is the type of object in the buffer.
            template <typename T = byte_t>
            T* get()const;

            /// @brief return a const pointer to the block of memory cast to the requested type.
            ///
            /// This should be used when the buffer already contains an object of the appropriate type.
            /// The memory is not changed by this call.
            ///          
            /// @tparam T is the type of object in the buffer.
            template <typename T = byte_t>
            const T* getConst()const;

            /// @brief Set the number of bytes in the buffer that contain valid data.
            /// @param used is the total number of bytes used in the buffer.
            /// @returns its argument for convenience.
            /// @throws runtime_error if used exceeds the buffer capacity.

            size_t setUsed(size_t used);
            /// @brief How many bytes in this buffer contain valid data?
            /// @param returns the number of bytes used.
            size_t getUsed() const;

            /// @brief construct a new object of type T in the buffer using placement new.
            /// @tparam T is the type of object to be constructed.
            /// @tparam ArgTypes are the types arguments to pass to the constructor.
            /// @param args are the actual arguments.
            template <typename T, typename... ArgTypes>
            T* construct(ArgTypes&&... args)
            {
                setUsed(sizeof(T));
                return new (get<T>()) T(std::forward<ArgTypes>(args)...);
            }

            /// @brief How many objects of type T can be added to the buffer.
            /// @tparam T is the type of object
            template <typename T = byte_t>
            size_t available() const;

            /// @brief Is there room for count additional objects of type T in the buffer?
            /// @tparam T is the type of object
            template <typename T = byte_t>
            bool needSpace(size_t count) const;

            /// @brief Increase the amount of space used inthe buffer.
            /// @tparam T is the type of object
            /// @param count is the number of T's to be added to the buffer.
            template <typename T = byte_t>
            size_t addUsed(size_t count);

            /// @brief Return the next available location in the buffer as a pointer to T.
            /// @tparam T is the type of object
            template <typename T = byte_t>
            T* getWritePosition()const;

            /// @brief Use the copy constructor to construct a new object of type T in the next available location in the buffer.
            /// @tparam T is the type of object
            /// @param object is the object to be copied.
            /// @returns a pointer to the newly copy-constructed object in the buffer.
            template <typename T = byte_t>
            T* appendNewCopy(const T & object);

            /// @brief Use a binary copy to initialize the next available location in the buffer.
            /// @tparam T is the type of object
            /// @param data is the object to be copied.
            /// @param count is the number of Ts to copy.
            /// @returns a pointer to the newly initialized data in the buffer.
            template <typename T = byte_t>
            T* appendBinaryCopy(const T * data, size_t count);

            /// @brief Associate a memory block from a MemoryBlockPool with this buffer.
            /// The block of data is general purpose.  It can be written to and reused as necessary.
            /// Normally this is only called once per buffer.  The buffer continues to use the same memory for its
            /// entire lifetime.  This is a typical use, not a requirement.
            /// @param pool The address of the pool containing ths buffer.
            /// @param capacity  The capacity of this buffer
            /// @param offset The offset to this buffer within the pool
            /// @param used The number of bytes used
            void set(MemoryBlockPool * pool, size_t capacity, size_t offset, size_t used = 0);

            /// @brief Undo a set.  Return the memory to the pool (if any), and make the buffer Invalid.
            void release();

            /// @brief Associate this memory block with one or two segments of memory contained in some other object.
            /// Note: "borrow" is a useful concept borrowed from Rust.
            ///
            /// Normally this method will be used when the data appears inside an external buffer used for some other purpose 
            /// for example a buffer read from a TCP stream or a file stream.  Because the message boundaries don't
            /// necessarily match buffer boundaries in this use case, borrow allows the data to appear in two separate
            /// chunks within the buffer.
            ///
            /// @param container The base address for the external buffer containing the memory.
            /// @param offset The offset to this buffer within the container
            /// @param used The number of bytes used
            /// @param offsetSplit The offset to the second chunk data in the external buffer.
            /// @param usedSplit The number of bytes used in the second chunk.
            void borrow(const byte_t * container, size_t offset, size_t used, size_t offsetSplit = 0, size_t usedSplit = 0);

            /// @brief Mark the buffer empty.
            void setEmpty();

            /// @brief Is the buffer empty?
            bool isEmpty()const;

            /// @brief Swap the contents of two buffers.
            /// Fast
            /// @param rhs is the buffer that will be swapped with *this
            /// @throws runtime_exception if either buffer is unsuitable for swapping.
            void swap(Buffer & rhs);

            /// @brief Move the data from one buffer to another, leaving the original buffer empty.
            /// Fast if both buffers are normal.
            /// Fast enough if the source buffer is borrowed.
            /// The target buffer must not be borrowed.
            /// After the move, the target buffer will be normal.
            /// @param target is the buffer to receive the data
            /// @throws runtime_exception if the target buffer is not suitable.
            void moveTo(Buffer & target);

            /// @brief Is this buffer normal or borrowed?
            bool isValid() const;
            /// @brief Is this buffer borrowed?
            bool isBorrowed() const;
            /// @brief Is this buffer contiguous (not split?)
            bool isContiguous() const;

            /// @brief Throw a runtime_exception if this is not a normal buffer
            /// For internal use, but you can use it if you feel the need.
            /// @param message to appear in the exception.
            void mustBeNormal(const char * message = "Operation cannot be applied to an immutable buffer.") const;

            /// @brief Throw a runtime_exception if this is not a valid buffer
            /// For internal use, but you can use it if you feel the need.
            void mustBeValid(const char * message = "Operation requires a valid buffer.") const;

            /// @brief Get the base address of the block of memory containing this buffers memory.
            /// NOTE: this is not an interesting function.  Do not use it.
            byte_t * getContainer()const;

            /// @brief Get the offset from the container's base address
            /// NOTE: this is not an interesting function.  Do not use it.
            size_t getOffset()const;

            /// @brief How is this buffer associated with its memory?
            Type getType() const;

            /// @brief Prepare a buffer for reuse -- make it Invalid.
            /// Warning:  This is not the method you want.  Call release() instead, or simply delete the Buffer. 
            /// This method should be private to be called only indirectly by a buffer owner, but
            /// I can't figure out how to do the appropriate friendship.  
            void reset();

        private:
            byte_t * container_;
            size_t capacity_;
            size_t offset_;
            size_t used_;
            size_t offsetSplit_;
            size_t usedSplit_;
            Type type_;
        };

        inline
        void Buffer::borrow(const byte_t * container, size_t offset, size_t used, size_t offsetSplit, size_t usedSplit)
        {
            int todo_ReleaseNormalIfNecessary;

            // setting type=Borrowed enforces the constness.
            container_ = const_cast<byte_t *>(container);
            capacity_ = used;
            offset_ = offset;
            used_ = used;
            offsetSplit_ = offsetSplit;
            usedSplit_ = usedSplit;
            type_ = Borrowed;
        }

        inline
        size_t Buffer::setUsed(size_t used)
        {
            mustBeNormal("Buffer::setUsed: Invalid operation on buffer.");
            if(used > capacity_)
            {
                throw std::runtime_error("Buffer used > capacity");
            }
            used_ = used;
            return used_;
        }

        inline
        void Buffer::setEmpty()
        {
            used_ = 0;
            usedSplit_ = 0;
        }

        inline
        size_t Buffer::getUsed() const
        {
            return used_ + usedSplit_;
        }

        inline
        bool Buffer::isEmpty()const
        {
            return used_ + usedSplit_ == 0;
        }

        inline
        Buffer::Type Buffer::getType() const
        {
            return type_;
        }

        inline
        bool Buffer::isValid() const
        {
            return type_ != Invalid;
        }

        inline
        bool Buffer::isBorrowed() const
        {
            return type_ == Borrowed;
        }

        inline
        bool Buffer::isContiguous() const
        {
            return usedSplit_ == 0;
        }

        inline
        void Buffer::mustBeNormal(const char * message) const
        {
            if(type_ != Normal)
            {
                throw std::runtime_error(message);
            }
        }

        inline
        void Buffer::mustBeValid(const char * message) const
        {
            if(type_ == Invalid)
            {
                throw std::runtime_error(message);
            }
        }

        inline
        void Buffer::swap(Buffer & rhs)
        {
            if(type_ == Borrowed || rhs.type_ == Borrowed)
            {
                throw std::runtime_error("Buffer::swap: Invalid operation on Borrowed buffer.");
            }
            std::swap(container_, rhs.container_);
            std::swap(capacity_, rhs.capacity_);
            std::swap(offset_, rhs.offset_);
            std::swap(used_, rhs.used_);
            std::swap(type_, rhs.type_);
            // no need to swap the split offset and used.  Normal buffers cannot be split.
        }

        inline
        void Buffer::moveTo(Buffer & rhs)
        {
            mustBeValid("Buffer::moveTo: source buffer is invalid.");
            rhs.mustBeNormal("Buffer::moveTo: target buffer not suitable for operation." );
            if(type_ == Borrowed)
            {
                if(used_ + usedSplit_ > rhs.capacity_)
                {
                    throw std::runtime_error("Move to: target buffer too small");
                }
                auto rhsBuffer = rhs.get();
                memcpy(rhsBuffer, getConst(), used_);
                if(usedSplit_ != 0)
                {
                    rhsBuffer += used_;
                    memcpy(rhsBuffer, container_ + offsetSplit_, usedSplit_);
                }
            }
            else
            {
                swap(rhs);
            }
            used_ = 0;
        }

        template <typename T>
        T* Buffer::get()const
        {
            mustBeNormal("Buffer::get: Invalid access to buffer");
            return reinterpret_cast<T *>(container_ + offset_);
        }

        template <typename T>
        const T* Buffer::getConst()const
        {
            mustBeValid();
            return reinterpret_cast<T *>(container_ + offset_);
        }

        template <typename T>
        size_t Buffer::available() const
        {
            return (capacity_ - used_) / sizeof(T);
        }

        template <typename T>
        bool Buffer::needSpace(size_t count) const
        {
            return available<T>() >= count;
        }

        template <typename T>
        size_t Buffer::addUsed(size_t count)
        {
            mustBeNormal("Buffer::addUsed: Invalid access to buffer.");
            return setUsed(used_ + count * sizeof(T));
        }

        template <typename T>
        T* Buffer::getWritePosition()const
        {
            mustBeNormal("Buffer::getWritePosition: Invalid access to buffer.");
            return reinterpret_cast<T *>(container_ + offset_ + used_);
        }

        template <typename T>
        T * Buffer::appendNewCopy(const T & data)
        {
            auto position = getWritePosition(); 
            // add used before writing to the buffer to catch overruns first.
            addUsed<T>(1);
            return new (position) T(data);
        }

        template <typename T>
        T * Buffer::appendBinaryCopy(const T * data, size_t count)
        {
            auto position = getWritePosition(); 
            // add used before writing to the buffer to catch overruns first.
            addUsed<T>(count);
            std::memcpy(position, data, count * sizeof(T));
            return reinterpret_cast<T *>(position);
        }
    }
}
