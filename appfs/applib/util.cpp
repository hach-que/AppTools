/*

Code file for Util.

This class contains low-level utility functions which do not
suit any other existing class.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                28th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#include <string>
#include <iostream>
#include <fstream>
#include "util.h"
#include "logging.h"
#include "blockstream.h"
#include <sys/types.h>
#include <sys/stat.h>

namespace AppLib
{
	namespace LowLevel
	{
		void Util::seekp_ex(LowLevel::BlockStream * fd, std::streampos pos)
		{
			fd->clear();
			fd->seekp(pos);
			if (fd->fail())
			{
				fd->clear();
				// This means that pos is beyond the current filesize.
				// We need to expand the file by writing NUL characters
				// until fd->tellp is equal to pos.
				fd->seekp(0, std::ios::end);
				if (fd->fail())
				{
					Logging::showErrorW("Unable to expand the provided file desciptor to the desired position.");
				}
				char zero = 0;
				while (fd->tellp() != pos)
				{
					fd->write(&zero, 1);
				}
			}
		}

		bool Util::fileExists(std::string filename)
		{
			struct stat file_info;
			return (stat(filename.c_str(), &file_info) == 0);
		}
	}
}
