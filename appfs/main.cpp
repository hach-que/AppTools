/*

Code file for main.cpp

This file is the main entry point for the AppFS / AppMount
programs.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "applib/logging.h"
#include "config.h"
#include "funcdefs.h"

int main(int argc, char *argv[])
{
#ifdef APPMOUNT
	return appmount_start(argc, argv);
#else
	return appfs_start(argc, argv);
#endif
}