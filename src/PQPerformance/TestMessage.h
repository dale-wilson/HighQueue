#pragma once

namespace MPass
{
    namespace ProntoQueue
    {
        struct TestMessage
        {
            uint32_t producerNumber_;
            uint64_t messageNumber_;
            static const size_t ExtraCount = 10;
            uint64_t extra_[ExtraCount];
            TestMessage(uint32_t producerNumber, uint64_t messageNumber)
                : producerNumber_(producerNumber)
                , messageNumber_(messageNumber)
            {
                for(size_t nExtra = 0; nExtra < ExtraCount; ++nExtra)
                {
                    extra_[nExtra] = nExtra;
                }
            }

            uint64_t touch() const
            {
                uint64_t value = messageNumber_;
                for(size_t nExtra = 0; nExtra < ExtraCount; ++nExtra)
                {
                    value += extra_[nExtra];
                }
                return value;
            }
        };
    }
}

