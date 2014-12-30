// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <InfiniteVector/details/MemoryBlockPoolFwd.h>
namespace MPass
{
    namespace InfiniteVector
    {
        /// @brief A handle for a block of memory
        class Message
        {
        public:
            enum Type
            {
                Normal,
                Borrowed,
                Invalid
            };
            const static size_t NO_POOL = ~size_t(0);

            /// @brief construct an empty (Invalid) message.
            Message();

            ~Message();

            /// @brief return a pointer to the block of memory cast to the requested type.  
            ///
            /// This should be used when the message already contains an object of the appropriate type.
            /// or when the caller plans to construct/initialize the contents of the message.
            /// The memory is not changed by this call, 
            ///          
            /// @tparam T is the type of object in the message.
            template <typename T = byte_t>
            T* get()const;

            /// @brief return a const pointer to the block of memory cast to the requested type.
            ///
            /// This should be used when the message already contains an object of the appropriate type.
            /// The memory is not changed by this call.
            ///          
            /// @tparam T is the type of object in the message.
            template <typename T = byte_t>
            const T* getConst()const;

            /// @brief Set the number of bytes in the message that contain valid data.
            /// @param used is the total number of bytes used in the message.
            /// @returns its argument for convenience.
            /// @throws runtime_error if used exceeds the message capacity.

            size_t setUsed(size_t used);
            /// @brief How many bytes in this message contain valid data?
            /// @param returns the number of bytes used.
            size_t getUsed() const;

            /// @brief construct a new object of type T in the message using placement new.
            /// @tparam T is the type of object to be constructed.
            /// @tparam ArgTypes are the types arguments to pass to the constructor.
            /// @param args are the actual arguments.
            template <typename T, typename... ArgTypes>
            T* construct(ArgTypes&&... args)
            {
                setUsed(sizeof(T));
                return new (get<T>()) T(std::forward<ArgTypes>(args)...);
            }

            /// @brief How many objects of type T can be added to the message.
            /// @tparam T is the type of object
            template <typename T = byte_t>
            size_t available() const;

            /// @brief Is there room for count additional objects of type T in the message?
            /// @tparam T is the type of object
            template <typename T = byte_t>
            bool needSpace(size_t count) const;

            /// @brief Increase the amount of space used inthe message.
            /// @tparam T is the type of object
            /// @param count is the number of T's to be added to the message.
            template <typename T = byte_t>
            size_t addUsed(size_t count);

            /// @brief Return the next available location in the message as a pointer to T.
            /// @tparam T is the type of object
            template <typename T = byte_t>
            T* getWritePosition()const;

            /// @brief Use the copy constructor to construct a new object of type T in the next available location in the message.
            /// @tparam T is the type of object
            /// @param object is the object to be copied.
            /// @returns a pointer to the newly copy-constructed object in the message.
            template <typename T = byte_t>
            T* appendNewCopy(const T & object);

            /// @brief Use a binary copy to initialize the next available location in the message.
            /// @tparam T is the type of object
            /// @param data is the object to be copied.
            /// @param count is the number of Ts to copy.
            /// @returns a pointer to the newly initialized data in the message.
            template <typename T = byte_t>
            T* appendBinaryCopy(const T * data, size_t count);

            /// @brief Associate a memory block from a MemoryBlockPool with this message.
            /// The block of data is general purpose.  It can be written to and reused as necessary.
            /// Normally this is only called once per message.  The message continues to use the same memory for its
            /// entire lifetime.  This is a typical use, not a requirement.
            /// @param pool The address of the pool containing ths message.
            /// @param capacity  The capacity of this message
            /// @param offset The offset to this message within the pool
            /// @param used The number of bytes used
            void set(MemoryBlockPool * pool, size_t capacity, size_t offset, size_t used = 0);

            /// @brief Undo a set.  Return the memory to the pool (if any), and make the message Invalid.
            void release();

            /// @brief Associate this memory block with one or two segments of memory contained in some other object.
            /// Note: "borrow" is a useful concept borrowed from Rust.
            ///
            /// Normally this method will be used when the data appears inside an external buffeR used for some other purpose 
            /// for example a buffeR read from a TCP stream or a file stream.  Because the message boundaries don't
            /// necessarily match buffeR boundaries in this use case, borrow allows the data to appear in two separate
            /// chunks within the same buffeR.
            ///
            /// @param container The base address for the external buffeR containing the memory.
            /// @param offset The offset to this buffeR within the container
            /// @param used The number of bytes used
            /// @param offsetSplit The offset to the second chunk data in the external buffeR.
            /// @param usedSplit The number of bytes used in the second chunk.
            void borrow(const byte_t * container, size_t offset, size_t used, size_t offsetSplit = 0, size_t usedSplit = 0);

            /// @brief Mark the message empty.
            void setEmpty();

            /// @brief Is the message empty?
            bool isEmpty()const;

            /// @brief Swap the contents of two messages.
            /// Fast
            /// @param rhs is the message that will be swapped with *this
            /// @throws runtime_exception if either message is unsuitable for swapping.
            void swap(Message & rhs);

            /// @brief Move the data from one message to another, leaving the original message empty.
            /// Fast if both messages are normal.
            /// Fast enough if the source message is borrowed.
            /// The target message must not be borrowed.
            /// After the move, the target message will be normal.
            /// @param target is the message to receive the data
            /// @throws runtime_exception if the target message is not suitable.
            void moveTo(Message & target);

            /// @brief Is this message normal or borrowed?
            bool isValid() const;
            /// @brief Is this message borrowed?
            bool isBorrowed() const;
            /// @brief Is this message contiguous (not split?)
            bool isContiguous() const;

            /// @brief Throw a runtime_exception if this is not a normal message
            /// For internal use, but you can use it if you feel the need.
            /// @param message to appear in the exception.
            void mustBeNormal(const char * message = "Operation cannot be applied to an immutable message.") const;

            /// @brief Throw a runtime_exception if this is not a valid message
            /// For internal use, but you can use it if you feel the need.
            void mustBeValid(const char * message = "Operation requires a valid message.") const;

            /// @brief Get the base address of the block of memory containing this message's memory.
            /// NOTE: this is not an interesting function.  Do not use it.
            byte_t * getContainer()const;

            /// @brief Get the offset from the container's base address
            /// NOTE: this is not an interesting function.  Do not use it.
            size_t getOffset()const;

            /// @brief How is this message associated with its memory?
            Type getType() const;

            /// @brief Prepare a message for reuse -- make it Invalid.
            /// Warning:  This is not the method you want.  Call release() instead, or simply delete the Message. 
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
        void Message::borrow(const byte_t * container, size_t offset, size_t used, size_t offsetSplit, size_t usedSplit)
        {
            if(type_ == Normal)
            {
                throw std::runtime_error("Message: Can't borrow a normal buffer.");
            }

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
        size_t Message::setUsed(size_t used)
        {
            mustBeNormal("Message::setUsed: Invalid operation on message.");
            if(used > capacity_)
            {
                throw std::runtime_error("Message used > capacity");
            }
            used_ = used;
            return used_;
        }

        inline
        void Message::setEmpty()
        {
            used_ = 0;
            usedSplit_ = 0;
        }

        inline
        size_t Message::getUsed() const
        {
            return used_ + usedSplit_;
        }

        inline
        bool Message::isEmpty()const
        {
            return used_ + usedSplit_ == 0;
        }

        inline
        Message::Type Message::getType() const
        {
            return type_;
        }

        inline
        bool Message::isValid() const
        {
            return type_ != Invalid;
        }

        inline
        bool Message::isBorrowed() const
        {
            return type_ == Borrowed;
        }

        inline
        bool Message::isContiguous() const
        {
            return usedSplit_ == 0;
        }

        inline
        void Message::mustBeNormal(const char * message) const
        {
            if(type_ != Normal)
            {
                throw std::runtime_error(message);
            }
        }

        inline
        void Message::mustBeValid(const char * message) const
        {
            if(type_ == Invalid)
            {
                throw std::runtime_error(message);
            }
        }

        inline
        void Message::swap(Message & rhs)
        {
            if(type_ == Borrowed || rhs.type_ == Borrowed)
            {
                throw std::runtime_error("Message::swap: Invalid operation on Borrowed message.");
            }
            std::swap(container_, rhs.container_);
            std::swap(capacity_, rhs.capacity_);
            std::swap(offset_, rhs.offset_);
            std::swap(used_, rhs.used_);
            std::swap(type_, rhs.type_);
            // no need to swap the split offset and used.  Normal messages cannot be split.
        }

        inline
        void Message::moveTo(Message & rhs)
        {
            mustBeValid("Message::moveTo: source message is invalid.");
            rhs.mustBeNormal("Message::moveTo: target message not suitable for operation." );
            if(type_ == Borrowed)
            {
                if(used_ + usedSplit_ > rhs.capacity_)
                {
                    throw std::runtime_error("Move to: target message too small");
                }
                auto rhsMessage = rhs.get();
                memcpy(rhsMessage, getConst(), used_);
                if(usedSplit_ != 0)
                {
                    rhsMessage += used_;
                    memcpy(rhsMessage, container_ + offsetSplit_, usedSplit_);
                }
            }
            else
            {
                swap(rhs);
            }
            used_ = 0;
        }

        template <typename T>
        T* Message::get()const
        {
            mustBeNormal("Message::get: Invalid access to message");
            return reinterpret_cast<T *>(container_ + offset_);
        }

        template <typename T>
        const T* Message::getConst()const
        {
            mustBeValid();
            return reinterpret_cast<T *>(container_ + offset_);
        }

        template <typename T>
        size_t Message::available() const
        {
            return (capacity_ - used_) / sizeof(T);
        }

        template <typename T>
        bool Message::needSpace(size_t count) const
        {
            return available<T>() >= count;
        }

        template <typename T>
        size_t Message::addUsed(size_t count)
        {
            mustBeNormal("Message::addUsed: Invalid access to message.");
            return setUsed(used_ + count * sizeof(T));
        }

        template <typename T>
        T* Message::getWritePosition()const
        {
            mustBeNormal("Message::getWritePosition: Invalid access to message.");
            return reinterpret_cast<T *>(container_ + offset_ + used_);
        }

        template <typename T>
        T * Message::appendNewCopy(const T & data)
        {
            auto position = getWritePosition(); 
            // add used before writing to the message to catch overruns first.
            addUsed<T>(1);
            return new (position) T(data);
        }

        template <typename T>
        T * Message::appendBinaryCopy(const T * data, size_t count)
        {
            auto position = getWritePosition(); 
            // add used before writing to the message to catch overruns first.
            addUsed<T>(count);
            std::memcpy(position, data, count * sizeof(T));
            return reinterpret_cast<T *>(position);
        }
    }
}
