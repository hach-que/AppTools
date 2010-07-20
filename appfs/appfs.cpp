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
#include "funcdefs.h"

int appfs_start(int argc, char *argv[])
{
	const char * disk_path = NULL;
	const char * mount_path = NULL;

	// Set the application name.
	AppLib::Logging::setApplicationName(std::string("appfs"));

	// AppFS needs a temporary location for the mountpoint.
	char mount_path_template[] = "/tmp/appfs_mount.XXXXXX";
	mount_path = mkdtemp(mount_path_template);
	if (mount_path == NULL)
	{
		AppLib::Logging::showErrorW("Unable to create temporary directory for mounting.");
		return 1;
	}

	AppLib::Logging::showErrorW("This software is not implemented ;)");
	return 0;
}

void appfs_continue()
{
	// Execution continues at this point when the filesystem is mounted.
}