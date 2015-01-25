// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/StagePch.h>

#include "BoostPropertyTreeConfiguration.h"
#include <Common/Log.h>
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

bool BoostPropertyTreeChildren::has() const
{
    return position_ != ptree_.end();
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
    auto result = std::make_shared<BoostPropertyTreeNode>(position_->first, position_->second);
    return result;
}

///////////////
// NODE

BoostPropertyTreeNode::BoostPropertyTreeNode()
{
}

BoostPropertyTreeNode::BoostPropertyTreeNode(const std::string & name, boost::property_tree::ptree ptree)
    : name_(name)
    , ptree_(ptree)
{
    LogTrace("BoostPropertyTreeNode: construct node: " << name_ );
}


BoostPropertyTreeNode::~BoostPropertyTreeNode()
{
}


void BoostPropertyTreeNode::loadJson(std::istream & propertyFile, const std::string & name)
{
    LogTrace("BoostPropertyTreeNode: Load Json from stream: " << name);
    name_ = name;
	boost::property_tree::json_parser::read_json(propertyFile, ptree_);
}

void BoostPropertyTreeNode::loadJson(const std::string & propertyFileName)
{
    LogTrace("BoostPropertyTreeNode: Load Json from file: " << propertyFileName);
    name_ = propertyFileName;
	boost::property_tree::json_parser::read_json(propertyFileName, ptree_);
}


ConfigurationChildrenPtr BoostPropertyTreeNode::getChildren()
{
    auto result = std::make_shared<BoostPropertyTreeChildren>(ptree_);
    return result;
}


std::string BoostPropertyTreeNode::getName()
{
    return name_;
}

bool BoostPropertyTreeNode::getValue(std::string & value, const std::string & defaultValue)const
{
    return getTypedValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(int64_t & value, int64_t defaultValue) const
{
    return getTypedValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(uint64_t & value, uint64_t defaultValue) const
{
    return getTypedValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(double & value, double defaultValue) const
{
    return getTypedValue(value, defaultValue);
}

bool BoostPropertyTreeNode::getValue(bool & value, bool defaultValue) const
{
    std::string str;
    if(getValue(str))
    {
        // this is terribly English-centric. Sorry....
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        if(str == "1" || str == "T" || str == "TRUE" || str == "Y" || str == "YES")
        {
            value = true;
            return true;
        }
        else if(str == "0" || str == "F" || str == "FALSE" || str == "N" || str == "NO")
        {
            value = false;
            return true;
        }
    }
    value = defaultValue;
    return false;
}
