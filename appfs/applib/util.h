/*

Header file for Util.

This class contains low-level utility functions which do not
suit any other existing class.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                28th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#ifndef CLASS_UTIL
#define CLASS_UTIL

#include <string>
#include <iostream>
#include <fstream>

namespace AppLib
{
	namespace LowLevel
	{
		class Util
		{
			public:
				static void seekp_ex(std::iostream * fd, std::streampos pos);
		};
	}
}

#endif
