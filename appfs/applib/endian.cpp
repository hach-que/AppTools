/*

Code file for Endian.

This class is used to convert memory data on machines that are 
not little-endian based.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                29th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#include <string>
#include <iostream>
#include <fstream>
#include "endian.h"

namespace AppLib
{
	namespace LowLevel
	{
		bool Endian::little_endian = true;

		void Endian::detectEndianness()
		{
			uint16_t test_int = 1;
			char * test_int_raw = reinterpret_cast<char*>(&test_int);
			Endian::little_endian = (test_int_raw[0] == 1);
		}

		void Endian::doR(std::iostream * fd, char * data, unsigned int size)
		{
			if (Endian::little_endian)
				fd->read(data, size);
			else
			{
				char* dStorage = (char*)malloc(size);
				fd->read(dStorage, size);
				for (unsigned int i = 0; i < size; i += 1)
				{
					data[i] = dStorage[size - i];
				}
			}
		}

		void Endian::doW(std::iostream * fd, char * data, unsigned int size)
		{
			if (Endian::little_endian)
				fd->write(data, size);
			else
			{
				char* dStorage = (char*)malloc(size);
				for (unsigned int i = 0; i < size; i += 1)
				{
					dStorage[size - i] = data[i];
				}
				fd->write(dStorage, size);
			}
		}

		void Endian::doW(std::iostream * fd, const char * data, unsigned int size)
		{
			Endian::doW(fd, const_cast<char*>(data), size);
		}
	}
}