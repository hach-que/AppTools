/*

Code file for appfs.cpp

This file is the code file for AppFS.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "applib/logging.h"
#include "config.h"

int appfs(int argc, char *argv[])
{
	AppLib::Logging::setApplicationName(std::string("appfs"));
	AppLib::Logging::showErrorW("This software is not implemented ;)");
	return 0;
}
