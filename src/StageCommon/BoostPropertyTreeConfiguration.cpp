// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/StagePch.h>

#include "BoostPropertyTreeConfiguration.h"

#include <boost/property_tree/json_parser.hpp>

using namespace HighQueue;
using namespace Stages;

BoostPropertyTreeChildren::BoostPropertyTreeChildren(boost::property_tree::ptree & ptree)
    : ptree_(ptree)
    , position_(ptree_.begin())
{
}

BoostPropertyTreeChildren::BoostPropertyTreeChildren(const BoostPropertyTreeChildren & rhs)
    : ptree_(rhs.ptree_)
    , position_(ptree_.begin())
{
}

BoostPropertyTreeChildren::~BoostPropertyTreeChildren()
{
}

bool BoostPropertyTreeChildren::first()
{
    position_ = ptree_.begin();
    return position_ != ptree_.end();
}

bool BoostPropertyTreeChildren::next()
{
    if(position_ != ptree_.end())
    {
        ++position_;
    }
    return position_ != ptree_.end();
}

ConfigurationNodePtr BoostPropertyTreeChildren::getChild()
{
    boost::property_tree::ptree & child = position_->second;
    auto result = std::make_shared<BoostPropertyTreeNode>(child);
    return result;
}

///////////////
// NODE

BoostPropertyTreeNode::BoostPropertyTreeNode()
{
}

BoostPropertyTreeNode::BoostPropertyTreeNode(boost::property_tree::ptree ptree)
	:ptree_(ptree)
{
}


BoostPropertyTreeNode::~BoostPropertyTreeNode()
{
}


void BoostPropertyTreeNode::loadJson(std::istream & propertyFile)
{
	boost::property_tree::json_parser::read_json(propertyFile, ptree_);
}

void BoostPropertyTreeNode::loadJson(const std::string & propertyFileName)
{
	boost::property_tree::json_parser::read_json(propertyFileName, ptree_);
}


ConfigurationChildrenPtr BoostPropertyTreeNode::getChildren()
{
    auto result = std::make_shared<BoostPropertyTreeChildren>(ptree_);
    return result;
}


std::string BoostPropertyTreeNode::getName()
{
	return ptree_.data();
}

bool BoostPropertyTreeNode::getValue(std::string & value, const std::string & defaultValue)const
{
    auto v = ptree_.get_value_optional<std::string>();
    if(v)
    {
        value = v.value();
        return true;
    }
    return false;
}

bool BoostPropertyTreeNode::getValue(int64_t & value, int64_t defaultValue) const
{
    return getValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(uint64_t & value, uint64_t defaultValue) const
{
    return getValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(double & value, double defaultValue) const
{
    return getValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(bool & value, bool defaultValue) const
{
    return getValue(value, defaultValue);
}
