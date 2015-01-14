#pragma once

namespace HighQueue
{
    template<size_t ExtraCount = 0>
    struct TestMessage
    {
        enum Indexes : uint32_t
        {
            MessageNumber = 0,
            ProducerNumber,
            ExtraStart
        };

        uint32_t message_[ExtraCount + ExtraStart];
        inline
        TestMessage(uint32_t producerNumber, uint32_t messageNumber)
        {
            message_[MessageNumber] = messageNumber;
            message_[ProducerNumber] = producerNumber;
            for(uint32_t nExtra = ExtraStart; nExtra < ExtraStart + ExtraCount; ++nExtra)
            {
                message_[nExtra] = nExtra;
            }
        }

        inline
        uint32_t messageNumber() const
        {
            return message_[MessageNumber];
        }

        inline
        uint32_t producerNumber() const
        {
            return message_[ProducerNumber];
        }

        inline
        uint32_t touch() const
        {
            uint32_t value = 0;
            for(uint32_t nExtra = 0; nExtra < ExtraStart + ExtraCount; ++nExtra)
            {
                value += message_[nExtra];
            }
            return value;
        }
    };
}


