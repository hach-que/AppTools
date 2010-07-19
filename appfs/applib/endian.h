/*

Header file for Endian.

This class is used to convert memory data on machines that are 
not little-endian based.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                29th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#ifndef CLASS_ENDIAN
#define CLASS_ENDIAN

#include <string>
#include <iostream>
#include <fstream>

namespace AppLib
{
	namespace LowLevel
	{
		class Endian
		{
			public:
				static bool little_endian;
				static void detectEndianness();
				static void doR(std::iostream * fd, char * data, unsigned int size);
				static void doW(std::iostream * fd, char * data, unsigned int size);
				static void doW(std::iostream * fd, const char * data, unsigned int size);
		};
	}
}

#endif