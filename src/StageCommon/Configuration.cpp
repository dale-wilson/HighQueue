// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/StagePch.h>

#include "Configuration.h"

using namespace HighQueue;
using namespace Stages;

ConfigurationIterator::~ConfigurationIterator()
{
}

ConfigurationNode::~ConfigurationNode()
{

}

bool ConfigurationNode::getValue(int64_t & value, int64_t defaultValue) const
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

bool ConfigurationNode::getValue(uint64_t & value, uint64_t defaultValue) const
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

bool ConfigurationNode::getValue(double & value, double defaultValue) const
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

bool ConfigurationNode::getBool(bool & value, bool defaultValue) const
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

