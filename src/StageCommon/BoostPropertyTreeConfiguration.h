// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StageCommon/Configuration.h>

#include <boost/property_tree/ptree.hpp>

namespace HighQueue
{
    namespace Stages
    {

        class Stages_Export BoostPropertyTreeNode : public ConfigurationNode
        {
        public:
			BoostPropertyTreeNode();

			explicit BoostPropertyTreeNode(boost::property_tree::ptree ptree);
			
            /// @brief destruct
			virtual ~BoostPropertyTreeNode();

			void load(const std::istream & propertyFile);
			void load(const std::string & propertyFileName);


            /// @brief Get the name of this node.
			virtual std::string getName();

            /// @brief Typesave get of the value associated with this node as a string
            /// @param[out] value receives the value.
            /// @param defaultValue is assigned to value if this node does not have a string value.
			virtual bool getValue(std::string & value, const std::string & defaultValue = "") const;

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
			virtual bool getValue(double & value, double defaultValue.0L) const;

			/// @brief Typesave get of the value associated with this node as a string
			/// @param[out] value receives the value.
			/// @param defaultValue is assigned to value if this node does not have a string value.
			virtual bool getBool(bool & value, bool defaultValue) const;

			virtual ConfigurationIterator begin() const;
			virtual ConfigurationIterator end() const;

		private:
			boost::property_tree::ptree ptree_;
        };

   }
}