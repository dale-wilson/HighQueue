/// @file IvPositionEntry.h
#pragma once
#include "IvDefinitions.h"
namespace MPass
{
	namespace InfiniteVector
	{
		PRE_CACHE_ALIGN
		struct IvPositionEntry
		{
            Position position_;
            IvPositionEntry(PositionBase initialValue = 0)
                : position_(initialValue)
            {
            }

            operator Position &()
            {
                return position_;
            }

            operator const Position &() const
            {
                return position_;
            }
        } POST_CACHE_ALIGN;
	}
}