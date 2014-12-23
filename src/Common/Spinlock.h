#pragma once

namespace MPass
{
    class SpinLock
    {
    public:
        SpinLock()
        : target_(0u)
        {
            int todo;
        }
    private:
        uint32_t target_;
    };
}