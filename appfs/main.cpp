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

int appmount(int argc, char *argv[]);
int appfs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
#ifdef APPMOUNT
	return appmount(argc, argv);
#else
	return appfs(argc, argv);
#endif
}