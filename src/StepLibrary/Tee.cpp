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
    StepFactory::Registrar<Tee> registerStep("tee", "Dump messages to a file while forwarding them to the next Step");
    const std::string keyOutput = "output";
}


Tee::Tee()
    : out_(0)
{
}

void Tee::attachOutputStream(std::ostream * outputStream)
{
    out_ = outputStream;
}

std::ostream & Tee::usage(std::ostream & out) const
{
    out << "    " << keyOutput << ": The name of a file to which messages will be dumped. \"cout\" and \"cerr\" are recognized as stdout and stderr." << std::endl;
    return Step::usage(out);
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
    if(!out_)
    {
        if(outputName_ == "cout")
        {
            out_ = & std::cout;
            LogTrace("Tee processor will write to stdout");
        }
        else if(outputName_ == "cerr")
        {
            out_ = & std::cerr;
            LogTrace("Tee processor will write to stderr");
        }
        else if(outputName_ == "null")
        {
            out_ = 0;
        }
        else
        {
            outfile_.open(outputName_);
            if(outfile_.good())
            {
                out_ = & outfile_;
                LogTrace("Tee processor will write to file: " << outputName_);
            }
            else
            {
                LogError("Tee processor cannot open output file " << outputName_);
            }
        }
    }
}


void Tee::handle(Message & message)
{
    if(!stopping_ && out_ != 0)
    { 
        *out_ << "Message type: " << message.getType() 
            << " Sequence: " << message.getSequence() 
            << " Time: " << message.getTimestamp() << std::endl;
        hexDump(message.get(), message.getUsed());
        send(message);
    }
}

void Tee::finish()
{
    if(outfile_.is_open())
    {
        outfile_.close();
        out_ = 0;
    }
}
    
void Tee::hexDump(byte_t * message, size_t size)
{
    if (out_)
    {
        *out_ << std::hex << std::setfill('0');
        for (size_t position = 0; position < size; position += bytesPerLine)
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
