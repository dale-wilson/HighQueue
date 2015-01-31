#pragma once

namespace HighQueue
{
    template<size_t ExtraCount = 0>
    struct MockMessage
    {
        enum Indexes : uint32_t
        {
            SequenceNumber = 0,
            ProducerNumber,
            ExtraStart
        };

        uint32_t message_[ExtraCount + ExtraStart];
        inline
        MockMessage(uint32_t producerNumber, uint32_t sequenceNumber)
        {
            message_[SequenceNumber] = sequenceNumber;
            message_[ProducerNumber] = producerNumber;
            for(uint32_t nExtra = ExtraStart; nExtra < ExtraStart + ExtraCount; ++nExtra)
            {
                message_[nExtra] = nExtra;
            }
        }

        inline
        uint32_t getSequence() const
        {
            return message_[SequenceNumber];
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
    typedef MockMessage<1> SmallMockMessage;
    typedef MockMessage<20> MediumMockMessage;
    typedef MockMessage<200> LargeMockMessage;
}


