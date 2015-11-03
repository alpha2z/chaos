/*  The following is the Simplified BSD License
 * 
 * Copyright (c) 2008, Warren Kurt vonRoeschlaub
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *  * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer 
 *       in the documentation and/or other materials provided with the 
 *       distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "color.h"

namespace configlib
{

color::color(unsigned long value /* = 0 */)
: clvalue(value)
{
}

color::color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha /*= 0*/)
: clvalue(((unsigned long)alpha << 24) | ((unsigned long)red << 16) |
		  ((unsigned long)green << 8) | (unsigned long)blue) 
{
}

color::~color()
{
}

std::istream& operator>>(std::istream& is, color& object)
{
	char line[512];
	is.getline(line, 512);
	object.parse(line);
	return is;
}

std::ostream& operator<<(std::ostream& os, const color& object)
{
	// Write as "r g b" triple or "r g b a" quad if the alpha channel
	// is non-zero
	os << (int)((object.get_value() >> 16) & 0x000000FF) << " ";
	os << (int)((object.get_value() >> 8) & 0x000000FF) << " ";
	os << (int)(object.get_value() & 0x000000FF);
	if (object.get_value() & 0xFF000000)
		os << " " << (int)((object.get_value() >> 24) & 0x000000FF);
	return os;
}

void color::parse(const std::string& str)
{
	int channel = 0;
	unsigned long hold = 0;
	int base = 10;
	
	std::size_t pos = str.find_first_not_of(" \t");
	while (pos < str.length())
	{
		if ('#' == str[pos])
		{
			// Web style color
			base = 16;
			hold = 0;
			channel = -1;
		}
		else if (('x' == str[pos]) || ('X' == str[pos]))
			base = 16;
		else if ((' ' == str[pos]) || ('\t' == str[pos]))
		{
			push_parsing(channel, hold);
			if (-1 == channel)
				channel = 3; // Next channel is alpha
			else
				channel++;
			hold = 0;
			base = 10;
		}
		else if (('0' <= str[pos]) && ('9' >= str[pos]))
			hold = ((hold * base) - '0') + str[pos];
		else if (('A' <= str[pos]) && ('F' >= str[pos]))
			hold = ((hold * base) - 'A' + 10) + str[pos];
		else if (('a' <= str[pos]) && ('f' >= str[pos]))
			hold = ((hold * base) - 'a' + 10) + str[pos];
		
		pos++;
	}
	if (0 < hold)
		push_parsing(channel, hold);
}

void color::push_parsing(int channel, unsigned long hold)
{
	switch (channel)
	{
		case -1 : // All bits
			clvalue = hold;
			break;
			
		case 0 : // Red channel
			clvalue = (clvalue & 0xFF00FFFF) | (hold << 16);
			break;
			
		case 1 : // Green channel
			clvalue = (clvalue & 0xFFFF00FF) | (hold << 8);
			break;
			
		case 2 : // Blue channel
			clvalue = (clvalue & 0xFFFFFF00) | hold;
			break;
		
		case 3 : // Alpha channel
			clvalue = (clvalue & 0x00FFFFFF) | (hold << 24);
			break;
	}
}

}
