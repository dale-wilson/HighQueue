// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

namespace HighQueue
{
    namespace Steps
    {
        class PoolBuilder;
        typedef std::shared_ptr<PoolBuilder> PoolBuilderPtr;
        class AsioBuilder;
        typedef std::shared_ptr<AsioBuilder> AsioBuilderPtr;
        class QueueBuilder;
        typedef std::shared_ptr<QueueBuilder> QueueBuilderPtr;
        class PipeBuilder;
        typedef std::shared_ptr<PipeBuilder> PipeBuilderPtr;
   }
}