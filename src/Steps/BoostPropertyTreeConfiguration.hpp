// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Configuration.hpp>

#include <boost/property_tree/ptree.hpp>

namespace HighQueue
{
    namespace Steps
    {

        class Steps_Export BoostPropertyTreeChildren: public ConfigurationChildren
        {
        public:
            explicit BoostPropertyTreeChildren(const boost::property_tree::ptree & ptree);
            BoostPropertyTreeChildren(const BoostPropertyTreeChildren & rhs);
            void set(boost::property_tree::ptree::iterator iterator);

            virtual ~BoostPropertyTreeChildren();
            virtual bool has() const override;
            virtual bool first() override;
            virtual bool next() override;
            virtual ConfigurationNodePtr getChild() override;

        private:
            boost::property_tree::ptree & ptree_;
            boost::property_tree::ptree::iterator position_;
        };


        class Steps_Export BoostPropertyTreeNode : public ConfigurationNode
        {
        public:
            BoostPropertyTreeNode();

            explicit BoostPropertyTreeNode(const std::string & name, boost::property_tree::ptree ptree);
            
            /// @brief destruct
            virtual ~BoostPropertyTreeNode();

            void loadJson(std::istream & propertyFile, const std::string & name = "json");
            void loadJson(const std::string & propertyFileName);

            virtual ConfigurationChildrenPtr getChildren() const override;
            virtual std::string getName()const override;
            virtual bool getValue(std::string & value, const std::string & defaultValue = "") const override;
            virtual bool getValue(int64_t & value, int64_t defaultValue = 0LL) const override;
            virtual bool getValue(uint64_t & value, uint64_t defaultValue = 0ULL) const override;
            virtual bool getValue(double & value, double defaultValue = 0L) const override;
            virtual bool getValue(bool & value, bool defaultValue) const override;
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