// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

namespace MPass
{
    namespace Buffers
    {
        /// @brief A handle for a block of memory
        class Buffer
        {
        public:
            /// @brief an interface for objects that own the memory contained in a buffer;
            class MemoryOwner
            {
            public:
                virtual void release(Buffer & buffer) = 0;
            };
            enum Type
            {
                Normal,
                Borrowed,
                Invalid
            };
            typedef std::shared_ptr<MemoryOwner> MemoryOwnerPtr;

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


#ifdef VISUAL_C_WAS_A_REAL_COMPILER
            template<class... _Types>
            _Ref_count_obj(_Types&&... _Args)
                : _Ref_count_base()
            {	// construct from argument list
                ::new ((void *)&_Storage) _Ty(_STD forward<_Types>(_Args)...);
            }

            /// @brief construct a new object of type T in the buffer using placement new.
            /// @tparam T is the type of object to be constructed.
            /// @tparam Args are the types arguments to pass to the constructor.
            /// @param args are the actual arguments.
            template <typename T, typename ...Args>
            T* construct(Args... & args)
            {
                auto result =  new (get<T>()) T (std::forward<Args>(arg)...);
                setUsed(sizeof(T));
                return result;
            }
#else
            /// @brief construct a new object of type T in the buffer using placement new.
            /// @tparam T is the type of object to be constructed.
            template <typename T>
            T* construct()
            {
                auto result = new (get<T>()) T();
                setUsed(sizeof(T));
                return result;
            }
            
            /// @brief Construct a new object of type T in the buffer using placement new.
            /// @tparam T is the type of object to be constructed.
            /// @tparam Arg is the type of an argument to pass to the contructor.
            /// @param arg1 is the actual arguments.
            template <typename T, typename Arg1>
            T* construct(Arg1 && arg1)
            {
                auto result = new (get<T>()) T(std::forward<Arg1>(arg1));
                setUsed(sizeof(T));
                return result;
            }

            /// @brief Construct a new object of type T in the buffer using placement new.
            /// @tparam T is the type of object to be constructed.
            /// @tparam Arg1 is the type of an argument to pass to the contructor.
            /// @tparam Arg2 is the type of an argument to pass to the contructor.
            /// @param arg1 is the actual arguments.
            /// @param arg2 is the actual arguments.
            template <typename T, typename Arg1, typename Arg2>
            T* construct(Arg1 && arg1, Arg2 && arg2)
            {
                auto result = new (get<T>()) T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2));
                setUsed(sizeof(T));
                return result;
            }
#endif            
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

            /// @brief Associate a memory block owned by a MemoryOwner with this buffer.
            /// The block of data is general purpose.  It can be written to and reused as necessary.
            /// Normally this is only called once per buffer.  The buffer continues to use the same memory for its
            /// entire lifetime.  This is a typical use, not a requirement.
            /// @param owner The owner to which this block will be returned when the buffer is released or deleted.  Null means no owner
            ///              Note that this is a shared pointer so the owner will live longer than the buffers it has populated.
            /// @param container The base address for the block containing ths buffer.
            /// @param capacity  The capacity of this buffer
            /// @param offset The offset to this buffer within the container
            /// @param used The number of bytes used
            /// @param type The type of buffer (Normal means memory owned by a MemoryOwner 
            void set(const MemoryOwnerPtr & owner, byte_t * container, size_t capacity, size_t offset, size_t used = 0, Type type = Normal);

            /// @brief Undo a set.  Return the memory to the owner, and make the buffer Invalid.
            void release();

            /// @brief Associate this memory block with one or two segments of memory contained in some other object.
            /// Note: "borrow" is a useful concept borrowe from Rust.
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
            MemoryOwnerPtr owner_;
            byte_t * container_;
            size_t capacity_;
            size_t offset_;
            size_t used_;
            size_t offsetSplit_;
            size_t usedSplit_;
            Type type_;
        };

        inline
        Buffer::Buffer()
        : owner_()
        , container_(0)
        , capacity_(0)
        , offset_(0)
        , used_(0)
        , offsetSplit_(0)
        , usedSplit_(0)
        , type_(Invalid)
        {
        }

        inline
        Buffer::~Buffer()
        {
            try{
                release();
            }
            catch(...) 
            {
                //ignore this (sorry!)
            }
        }

        inline
        void Buffer::set(const MemoryOwnerPtr & owner, byte_t * container, size_t capacity, size_t offset, size_t used, Type type)
        {
            owner_ = owner;
            container_ = container;
            capacity_ = (capacity == 0) ? used : capacity;
            offset_ = offset;
            used_ = used;
            offsetSplit_ = 0;
            usedSplit_ = 0;
            type_ = type;
        }

        inline
        void Buffer::borrow(const byte_t * container, size_t offset, size_t used, size_t offsetSplit, size_t usedSplit)
        {
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
        byte_t * Buffer::getContainer()const
        {
            mustBeValid();
            return container_;
        }

        inline
        size_t Buffer::getOffset()const
        {
            return offset_;
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
            owner_.swap(rhs.owner_);
            std::swap(container_, rhs.container_);
            std::swap(offset_, rhs.offset_);
            std::swap(used_, rhs.used_);
            // no need to swap the split offset and used.  Normal buffers cannot be split.
        }

        inline
        void Buffer::moveTo(Buffer & rhs)
        {
            mustBeValid("Buffer::moveTo: source buffer is invalid.");
            rhs.mustBeNormal("Buffer::moveTo: target buffer not suitable for operation." );
            if(type_ == Borrowed)
            {
                if(used_ > rhs.capacity_)
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
                owner_.swap(rhs.owner_);
                std::swap(container_,rhs.container_);
                std::swap(capacity_, rhs.capacity_);
                std::swap(offset_,rhs.offset_);
            }
            rhs.used_ = used_;
            used_ = 0;
        }

        inline
        void Buffer::release()
        {
            if(owner_)
            {
                owner_->release(*this);
            }
            else
            {
                reset();
            }
        }

        inline void Buffer::reset()
        {
            owner_.reset();
            container_ = 0;
            capacity_ = 0;
            offset_ = 0;
            used_ = 0;
            offsetSplit_ = 0;
            usedSplit_ = 0;
            type_ = Invalid;
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
            return (capacity_ - offset_ - used_) / sizeof(T);
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
