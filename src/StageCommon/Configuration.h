// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StageCommon/Stage_Export.h>

namespace HighQueue
{
    namespace Stages
    {

//		class ConfigurationNode;
		class ConfigurationIterator;

        class Stages_Export ConfigurationNode
        {
        public:
            /// @brief destruct
			virtual ~ConfigurationNode();

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
			virtual bool getBool(bool & value, bool defaultValue) const;

			virtual ConfigurationIterator begin() const = 0;
			virtual ConfigurationIterator end() const = 0;
        };

		class Stages_Export ConfigurationIterator
		{
		public:
			virtual ~ConfigurationIterator();
			virtual ConfigurationNode & operator->() = 0;
			virtual ConfigurationIterator & operator++() = 0;
		};

   }
}