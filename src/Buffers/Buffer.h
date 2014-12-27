#pragma once

namespace MPass
{
    namespace Buffers
    {
        class Buffer
        {
        public:
            // an interface for objects that own the memory contained in a buffer;
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

            Buffer();


            template <typename T = byte_t>
            T* get()const;
            template <typename T = byte_t>
            const T* getConst()const;


            size_t setUsed(size_t used);
            size_t getUsed() const;
#ifdef VISUAL_C_WAS_A_REAL_COMPILER
            template <typename T, typename ...Args>
            T* construct(Args... & args)
            {
                auto result =  new (get<T>()) T (std::forward<Args>(arg)...);
                setUsed(sizeof(T));
                return result;
            }
#else
            template <typename T>
            T* construct()
            {
                auto result = new (get<T>()) T();
                setUsed(sizeof(T));
                return result;
            }
            template <typename T, typename Arg1>
            T* construct(Arg1 && arg1)
            {
                auto result = new (get<T>()) T(std::forward<Arg1>(arg1));
                setUsed(sizeof(T));
                return result;
            }

            template <typename T, typename Arg1, typename Arg2>
            T* construct(Arg1 && arg1, Arg2 && arg2)
            {
                auto result = new (get<T>()) T(std::forward<Arg1>(arg1), std::forward(arg2));
                setUsed(sizeof(T));
                return result;
            }
#endif            
            template <typename T = byte_t>
            size_t available() const;

            template <typename T = byte_t>
            bool needSpace(size_t count) const;

            template <typename T = byte_t>
            size_t addUsed(size_t count);

            template <typename T = byte_t>
            T* getWritePosition()const;

            template <typename T = byte_t>
            size_t appendNewCopy(const T & object);

            template <typename T = byte_t>
            size_t appendBinaryCopy(const T * data, size_t count);


            void set(const MemoryOwnerPtr & owner, byte_t * container, size_t capacity, size_t offset, size_t used = 0, Type type = Normal);
            void reset();
            void borrow(const byte_t * container, size_t offset, size_t used, size_t offsetSplit = 0, size_t usedSplit = 0);
            void setEmpty();
            bool isEmpty()const;
            byte_t * getContainer()const;
            size_t getOffset()const;
            Type getType() const;
            void swap(Buffer & rhs);
            void moveTo(Buffer & rhs);

            bool isValid() const;
            bool isBorrowed() const;
            bool isSplit() const;
            void mustBeNormal(const char * = "Operation cannot be applied to an immutable buffer.") const;
            void mustBeValid(const char * message = "Operation requires a valid buffer.") const;

            void release();
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
        bool Buffer::isSplit() const
        {
            return type_ == Borrowed && usedSplit_ != 0;
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
                owner_.reset();
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
        size_t Buffer::appendNewCopy(const T & data)
        {
            auto position = getWritePosition(); 
            // add used before writing to the buffer to catch overruns first.
            addUsed<T>(1);
            new (position) T(data);
            return used_;
        }

        template <typename T>
        size_t Buffer::appendBinaryCopy(const T * data, size_t count)
        {
            auto position = getWritePosition(); 
            // add used before writing to the buffer to catch overruns first.
            addUsed<T>(count);
            std::memcpy(position, data, count * sizeof(T));
            return used_;
        }
    }
}
