/*

Header file for Environment.

This class provides functions for interacting with the
current environment, such as accessing environment variables
and validating the existance of binaries.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                6th August 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef CLASS_ENVIRONMENT
#define CLASS_ENVIRONMENT

#include "config.h"
#include <string>
#include <vector>

namespace AppLib
{
	class Environment
	{
		public:
			static std::vector<bool> searchForBinaries(std::vector<std::string> & binaries);
	};
}

#endif
