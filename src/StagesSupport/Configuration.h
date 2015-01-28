// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>

#include "ConfigurationFwd.h"

namespace HighQueue
{
    namespace Stages
    {
        /// @brief A collection of properties(child nodes)
        ///
        /// Note I tried to make this an iterator but various technical difficulties arose
        /// due to mixing templates and abstract methods.
        class Stages_Export ConfigurationChildren
        {
        public:
            virtual ~ConfigurationChildren();
            virtual bool has() const = 0;
            virtual bool first() = 0;
            virtual bool next() = 0;
            virtual ConfigurationNodePtr getChild() = 0;
        };

        /// @brief A Configuration entry
        ///
        /// Each entry has a name, a value, and a set of children.
        /// Any of these may be empty/missing.
        ///
        class Stages_Export ConfigurationNode
        {
        public:
            /// @brief destruct
            virtual ~ConfigurationNode();

            virtual ConfigurationChildrenPtr getChildren() = 0;

            /// @brief Get the name of this node.
            virtual std::string getName() = 0;

            /// @brief Typesave get of the value associated with this node as a string
            /// @param[out] value receives the value.
            /// @param defaultValue is assigned to value if this node does not have a string value.
            virtual bool getValue(std::string & value, const std::string & defaultValue = "") const = 0;

            /// @brief Typesave get of the value associated with this node as a string
            /// @param[out] value receives the value.
            /// @param defaultValue is assigned to value if this node does not have a string value.
            virtual bool getValue(int64_t & value, int64_t defaultValue = 0LL) const;

            /// @brief Typesave get of the value associated with this node as a string
            /// @param[out] value receives the value.
            /// @param defaultValue is assigned to value if this node does not have a string value.
            virtual bool getValue(uint64_t & value, uint64_t defaultValue = 0ULL) const;

            /// @brief Typesave get of the value associated with this node as a string
            /// @param[out] value receives the value.
            /// @param defaultValue is assigned to value if this node does not have a string value.
            virtual bool getValue(double & value, double defaultValue = 0.0L) const;

            /// @brief Typesave get of the value associated with this node as a string
            /// @param[out] value receives the value.
            /// @param defaultValue is assigned to value if this node does not have a string value.
            virtual bool getValue(bool & value, bool defaultValue = false) const;
        };
   }
}