// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

// make reverse iterator is defined in C++14
// this is a temporary measure until the standard is released and the compilers catch up.

namespace HighQueue
{
    template<typename Range>
    class ReverseRange
    {
    public:
        ReverseRange(Range & range)
        : range_(range)
        {}
        
        // do not accept temporary objects.
        ReverseRange(Range const &&) = delete;

        typename Range::reverse_iterator begin() const
        {
            return range_.rbegin();
        }
        typename Range::reverse_iterator end() const
        {
            return range_.rend();
        }
    private:
        Range & range_;
    };
}
