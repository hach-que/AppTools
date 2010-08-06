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
#include "applib/util.h"
#include "config.h"
#include "funcdefs.h"
#include <libgen.h>
#include <stdexcept>

int main(int argc, char *argv[])
{
#ifdef APPMOUNT
	return appmount_start(argc, argv);
#else
	// Determine whether or not we're starting stage 1 or stage 2.  This
	// depends on whether or not the filesize is less than or equal to
	// 1MB (in which case it'll be stage 2).
	struct stat scheck;
	const char* exepath = AppLib::LowLevel::Util::getProcessFilename();
	AppLib::Logging::setApplicationName("appfs"); // Just so our error messages
	                                              // have better context than 'apptools'.
	if (exepath == NULL)
	{
		AppLib::Logging::showErrorW("Unable to determine process's filename on this system.");
		AppLib::Logging::showErrorO("If you are running Linux, make sure /proc is mounted");
		AppLib::Logging::showErrorO("and that /proc/self/exe exists.");
		return 1;
	}
	if (stat(exepath, &scheck) < 0)
	{
		AppLib::Logging::showErrorW("Unable to detect size of bootstrap application.");
		return 1;
	}
	if (scheck.st_size <= 1024 * 1024)
	{
		// Stage 2.

		// First unlink ourselves from the filesystem.
		if (unlink(exepath) != 0)
		{
			AppLib::Logging::showWarningW("Unable to delete temporary bootstrap file.  You may");
			AppLib::Logging::showWarningO("have to clean it up yourself by deleting:");
			AppLib::Logging::showWarningO(" * %s", exepath);
		}

		// Then remove the directory that contains us (if it begins with appfs_stage2).
		int plen = strlen(exepath);
		char * drpath = (char*)malloc(plen + 1);
		for (int i = 0; i < plen + 1; i += 1)
			drpath[i] = 0;
		strcpy(drpath, exepath);
		dirname(drpath);
		std::string ddrpath = drpath;
		try
		{
			if (ddrpath.substr(ddrpath.length() - 20, 14) == "/appfs_stage2.")
			{
				if (rmdir(drpath) != 0)
				{
					AppLib::Logging::showWarningW("Unable to delete temporary bootstrap directory.  You may");
					AppLib::Logging::showWarningO("have to clean it up yourself by deleting:");
					AppLib::Logging::showWarningO(" * %s", drpath);
				}
			}
		}
		catch (std::out_of_range & err)
		{
			// Nothing to do...
		}

		return appfs_stage2(argc, argv);
	}
	else
	{
		// Stage 1.
		return appfs_stage1(argc, argv);
	}
#endif
}
