/*

Code file for main.cpp

This file is the main entry point for the AppFS / AppMount
programs.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                21th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "applib/logging.h"

int main(int argc, char *argv[])
{
	Logging.setApplicationName(std::string("appfs"));
	Logging.showErrorW("This software is not implemented ;)");
	return 0;
}
