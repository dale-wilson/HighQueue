// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "Tee.h"
#include <Steps/StepFactory.h>
#include <Steps/Configuration.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<Tee> registerStep("tee");
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

bool Tee::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyOutput)
    {
        return configuration.getValue(outputName_);
    }
    return Step::configureParameter(key, configuration);
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
