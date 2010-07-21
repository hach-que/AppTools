/*

Code file for appmount.cpp

This file is the code file for AppMount.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "applib/logging.h"
#include "applib/fuse.h"
#include "config.h"
#include "funcdefs.h"

int appmount_start(int argc, char *argv[])
{
	const char * disk_path = NULL;
	const char * mount_path = NULL;

	// Set the application name.
	AppLib::Logging::setApplicationName(std::string("appmount"));

	// Parse the arguments provided.
	struct arg_lit *is_readonly = arg_lit0("r", "read-only", "mount the file readonly");
	struct arg_file *disk_image = arg_file1(NULL, NULL, "diskimage", "the image to read the data from");
	struct arg_file *mount_point = arg_file1(NULL, NULL, "mountpoint", "the directory to mount the image to");
	struct arg_lit *show_help = arg_lit0("h", "help", "show the help message");
	struct arg_end *end = arg_end(4);
	void *argtable[] = {is_readonly, disk_image, mount_point, show_help, end};

	// Check to see if the argument definitions were allocated
	// correctly.
	if (arg_nullcheck(argtable))
	{
		AppLib::Logging::showErrorW("Insufficient memory.");
		return 1;
	}

	// Now parse the arguments.
	int nerrors = arg_parse(argc, argv, argtable);

	// Check to see if there were errors.
	if (nerrors > 0 && show_help->count == 0)
	{
		printf("Usage: appmount");
		arg_print_syntax(stdout, argtable, "\n");

		arg_print_errors(stdout, end, "appfs");
		return 1;
	}

	// Check to see if the user requested showing the help
	// message.
	if (show_help->count == 1)
	{
		printf("Usage: appmount");
		arg_print_syntax(stdout, argtable, "\n");

		printf("AppFS - An application storage filesystem.\n\n");
		arg_print_glossary(stdout, argtable, "    %-25s %s\n");
		return 0;
	}

	// Store the disk path and mount point the user has provided.
	disk_path = disk_image->filename[0];
	mount_path = mount_point->filename[0];

	// Lock the specified disk file.
	int lockedfd = open(disk_image->filename[0], O_RDWR);
	flock lock = { F_WRLCK, SEEK_SET, 0, 0, 0 };
	int lockres = fcntl(lockedfd, F_SETLK, &lock);
	int ret = 1;
	if (lockres != -1)
	{
		AppLib::FUSE::Mounter * mnt = new AppLib::FUSE::Mounter(
						disk_path,
						mount_path,
						true,
						appmount_continue
					);
		ret = mnt->getResult();
		return ret;
	}
	else
	{
		AppLib::Logging::showErrorW("Unable to lock image.  Check to make sure it's");
		AppLib::Logging::showErrorO("not already mounted.");
		return 1;
	}

	return 0;
}

void appmount_continue()
{
	// Execution continues at this point when the filesystem is mounted.
}
