/*

Code file for appmount.cpp

This file is the code file for AppMount.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "applib/logging.h"
#include "config.h"

int appmount(int argc, char *argv[])
{
	AppLib::Logging::setApplicationName(std::string("appmount"));
	AppLib::Logging::showErrorW("This software is not implemented ;)");
	return 0;
}
