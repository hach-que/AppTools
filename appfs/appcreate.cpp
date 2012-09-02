/*

Code file for appcreate.cpp

This file is the code file for AppCreate.

Last edited by: James Rhodes <jrhodes@redpointsoftware.com.au>
                5th February 2011

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "libapp/util.h"

int main(int argc, char *argv[])
{
	AppLib::Logging::setApplicationName("appcreate");
#ifdef DEBUG
	AppLib::Logging::debug = true;
#endif

	// Check arguments.
	if (argc != 2)
	{
		AppLib::Logging::showErrorW("Invalid arguments provided.");
		AppLib::Logging::showErrorO("Usage: appcreate <filename>");
		return 1;
	}

	std::cout << "Attempting to create '" << argv[1] << "' ... " << std::endl;

	// Create the file.
	if (!AppLib::LowLevel::Util::createPackage(argv[1], "Test Application", "1.0.0", "A test package.", "AppTools"))
	{
		std::cout << "Unable to create blank AppFS package '" << argv[1] << "'." << std::endl;
		return 1;
	}

	std::cout << "Package successfully created." << std::endl;
	return 0;
}
