// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/StagePch.h>

#include "BoostPropertyTreeConfiguration.h"

#include <boost/property_tree/json_parser.hpp>


using namespace HighQueue;
using namespace Stages;

namespace {
	class BoostPropertyTreeIterator
	{
	public:
		BoostPropertyTreeIterator(boost::property_tree::ptree::iterator iterator);
		BoostPropertyTreeIterator(const BoostPropertyTreeIterator & rhs);
        
        virtual ~BoostPropertyTreeIterator();
		virtual ConfigurationNode & operator->();
		virtual ConfigurationIterator & operator++();

		bool operator==(const & BoostPropertyTreeIterator rhs)const;

	private:
		boost::property_tree::ptree::iterator iterator_;

	};


	BoostPropertyTreeIterator::BoostPropertyTreeIterator(boost::property_tree::ptree::iterator & iterator)
		: iterator_(iterator)
	{
	}

	BoostPropertyTreeIterator::BoostPropertyTreeIterator(const BoostPropertyTreeIterator & rhs)
		: iterator_(rhs.iterator_)
	{
	}

	bool operator==(const & BoostPropertyTreeIterator rhs)const
	{
		return iterator_ == rhs.iterator_;
	}

	BoostPropertyTreeIterator::~BoostPropertyTreeIterator()
    {
    }

    ConfigurationNode & BoostPropertyTreeIterator::operator->()
    {
		return BoostPropertyTreeNode(*iterator_);
    }

    ConfigurationIterator & BoostPropertyTreeIterator::operator++()
    {
		++iterator_;
		return *this;
    }
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


void BoostPropertyTreeNode::load(const std::istream & propertyFile)
{
	boost::property_tree::json_parser::read_json(propertyFile, ptree_);
}

void BoostPropertyTreeNode::load(const std::string & propertyFileName)
{
	boost::property_tree::json_parser::read_json(propertyFileName, ptree_);
}

/// @brief Get the name of this node.
std::string BoostPropertyTreeNode::getName()
{
	return ptree_.data();
}

ConfigurationIterator BoostPropertyTreeNode::begin() const
{
	return BoostPropertyTreeIterator(ptree_.begin());
}

ConfigurationIterator BoostPropertyTreeNode::end() const
{
	return BoostPropertyTreeIterator(ptree_.end());
}



#if 0
/// @brief Typesave get of the value associated with this node as a string
/// @param[out] value receives the value.
/// @param defaultValue is assigned to value if this node does not have a string value.
bool getValue(std::string & value, const std::string & defaultValue = "") const
{

}



bool BoostPropertyTreeNode::getValue(int64_t & value, int64_t defaultValue) const
{
	std::string str;
	if (getValue(str))
	{
		try
		{
			value = boost::lexical_cast<int64_t>(str);
			return true;
		}
		catch (...)
		{
            // never mind
		}
	}
	value = defaultValue;
	return false;
}

bool BoostPropertyTreeNode::getValue(uint64_t & value, uint64_t defaultValue) const
{
	std::string str;
	if (getValue(str))
	{
		try
		{
			value = boost::lexical_cast<uint64_t>(str);
			return true;
		}
		catch (...)
		{
			// never mind
		}
	}
	value = defaultValue;
	return false;
}

bool BoostPropertyTreeNode::getValue(double & value, double defaultValue) const
{
	std::string str;
	if (getValue(str))
	{
		try
		{
			value = boost::lexical_cast<double>(str);
			return true;
		}
		catch (...)
		{
			// never mind
		}
	}
	value = defaultValue;
	return false;
}

bool BoostPropertyTreeNode::getBool(bool & value, bool defaultValue) const
{
	std::string str;
	if (getValue(str))
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		if (str == "y" || str == "yes" || str == "1" || str == "t" || str == "true")
		{
			value = true;
			return true;
		}
		else if (str == "n" || str == "no" || str == "0" || str == "f" || str == "false")
		{
			value = false;
			return true;
		}
	}
	value = defaultValue;
	return false;
}
#endif
