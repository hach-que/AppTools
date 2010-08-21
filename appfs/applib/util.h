/*

Header file for Util.

This class contains low-level utility functions which do not
suit any other existing class.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                28th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef CLASS_UTIL
#define CLASS_UTIL

#include "config.h"
#include <string>
#include <iostream>
#include <fstream>
#include "blockstream.h"

namespace AppLib
{
	namespace LowLevel
	{
		class Util
		{
			public:
				static void seekp_ex(LowLevel::BlockStream * fd, std::streampos pos);
				static bool fileExists(std::string filename);
				static void sanitizeArguments(char ** argv, int argc, std::string & command, int start);
				static bool extractBootstrap(std::string source, std::string dest);
				static const char* getProcessFilename();
				static bool createPackage(std::string path, const char* appname, const char* appver,
							const char* appdesc, const char* appauthor);
		};
	}
}

#endif
