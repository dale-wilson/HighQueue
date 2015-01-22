// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/Stage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class Tee: public Stage
        {
        public:
			/// @brief hard code the line width.
			static const size_t bytesPerLine = 16;

			/// @brief construct
            Tee();

			/// @brief Attach an ostream.  If none, don't dump the data.
			void attachOutputStream(std::ostream * outputStream);

            // implement stage methods
            virtual void handle(Message & message);

		private:
			void hexDump(byte_t * message, size_t size);
        private:
			std::ostream * out_;
        };

		inline
        Tee::Tee()
			: out_(0)
        {
        }

		inline
		void Tee::attachOutputStream(std::ostream * outputStream)
		{
			out_ = outputStream;
		}

		inline 
		void Tee::handle(Message & message)
        {
            if(!stopping_)
            { 
                LogTrace("Tee copy.");
				hexDump(message.get(), message.getUsed());
				send(message);
            }
        }
	
		inline
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
	}
}