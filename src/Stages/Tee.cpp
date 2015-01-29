// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "Tee.h"
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<Tee> registerTee("tee");
}

const std::string Tee::keyOutput = "output";

Tee::Tee()
    : out_(0)
{
    setName("Tee"); // default name
}

void Tee::attachOutputStream(std::ostream * outputStream)
{
    out_ = outputStream;
}

bool Tee::configure(const ConfigurationNodePtr & config)
{
    for(auto poolChildren = config->getChildren();
        poolChildren->has();
        poolChildren->next())
    {
        auto & parameter = poolChildren->getChild();
        auto & key = parameter->getName();

        if(key == keyName)
        {
            parameter->getValue(name_);
        }
        else if(key == keyOutput)
        {
            config->getValue(outputName_);
        }
        else
        {
            LogFatal("Unknown configuration parameter " << key << "  " << config->getName() << " "  << name_);
            return false;
        }
    }

    if(name_.empty())
    {
        LogFatal("Missing required parameter " << keyName << " for  " << config->getName() << ".");
        return false;
    }
    return true;
}

void Tee::start()
{
    if(outputName_ == "cout")
    {
        out_ = & std::cout;
    }
    else if(outputName_ == "cerr")
    {
        out_ = & std::cerr;
    }
    else
    {
        outfile_.open(outputName_);
        if(outfile_.good())
        {
            out_ = & outfile_;
        }
        else
        {
            LogError("Tee processor cannot open output file " << outputName_);
        }
    }
}


void Tee::handle(Message & message)
{
    if(!stopping_)
    { 
        LogTrace("Tee copy.");
        hexDump(message.get(), message.getUsed());
        send(message);
    }
}
    
void Tee::hexDump(byte_t * message, size_t size)
{
    if (out_)
    {
        *out_ << std::hex << std::setfill('0');
        for (size_t position = 0; position < size + bytesPerLine; position += bytesPerLine)
        {
            *out_ << std::setw(4) << position << ':';
            size_t pos = position;
            while (pos < position + bytesPerLine && pos < size)
            {
                *out_ << ' ' << std::setw(2) << (unsigned short)message[pos];
                ++pos;
            }
            while (pos < position + bytesPerLine)
            {
                *out_ << "   ";
                ++pos;
            }
            *out_ << ' ';
            pos = position;
            while (pos < position + bytesPerLine && pos < size)
            {
                char ch = message[pos];
                if (ch < ' ' || ch >= '\x7f') // technically should call isgraph()
                {
                    ch = '.';
                }
                *out_ << ch;
                ++pos;
            }
            *out_ << std::endl;
        }
        *out_ << std::setfill(' ') << std::dec;
    }
}
