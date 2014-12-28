#pragma once

namespace MPass
{
    namespace InfiniteVector
    {
        struct TestMessage
        {
            uint32_t producerNumber_;
            uint64_t messageNumber_;
            uint64_t extra_[3];
            TestMessage(uint32_t producerNumber, uint64_t messageNumber)
                : producerNumber_(producerNumber)
                , messageNumber_(messageNumber)
            {
                extra_[0] = extra_[1] = extra_[2] = messageNumber;
            }

            uint64_t touch() const
            {
                return messageNumber_ + extra_[0] + extra_[1] + extra_[2];
            }
        };
    }
}

