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

        class Stages_Export BoostPropertyTreeChildren: public ConfigurationChildren
        {
        public:
            explicit BoostPropertyTreeChildren(boost::property_tree::ptree & ptree);
            BoostPropertyTreeChildren(const BoostPropertyTreeChildren & rhs);
            void set(boost::property_tree::ptree::iterator iterator);

            virtual ~BoostPropertyTreeChildren();
            virtual bool has() const;
            virtual bool first();
            virtual bool next();
            virtual ConfigurationNodePtr getChild();

        private:
            boost::property_tree::ptree & ptree_;
            boost::property_tree::ptree::iterator position_;
        };


        class Stages_Export BoostPropertyTreeNode : public ConfigurationNode
        {
        public:
			BoostPropertyTreeNode();

			explicit BoostPropertyTreeNode(const std::string & name, boost::property_tree::ptree ptree);
			
            /// @brief destruct
			virtual ~BoostPropertyTreeNode();

			void loadJson(std::istream & propertyFile, const std::string & name = "json");
			void loadJson(const std::string & propertyFileName);

            virtual ConfigurationChildrenPtr getChildren();
			virtual std::string getName();
			virtual bool getValue(std::string & value, const std::string & defaultValue = "") const;
			virtual bool getValue(int64_t & value, int64_t defaultValue = 0LL) const;
			virtual bool getValue(uint64_t & value, uint64_t defaultValue = 0ULL) const;
			virtual bool getValue(double & value, double defaultValue = 0L) const;
            virtual bool getValue(bool & value, bool defaultValue) const;
        private:
            template <typename Type, typename DefaultType>
            bool getTypedValue(Type & value, DefaultType defaultValue) const;
		private:
            std::string name_;
			boost::property_tree::ptree ptree_;
        };

        template <typename Type, typename DefaultType>
        bool BoostPropertyTreeNode::getTypedValue(Type & value, DefaultType defaultValue) const
        {
            auto v = ptree_.get_value_optional<Type>();
            if(v)
            {
                value = v.value();
                return true;
            }
            value = defaultValue;
            return false;
        }
   }
}