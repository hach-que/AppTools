/*

AppFS - main.cpp

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                7th April 2010

-- LICENSE --

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#define SUPERBLOCK_OFFSET       0
#define FUSE_USE_VERSION	26

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <argtable2.h>
#include <string>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <fuse.h>
#include <fuse/fuse_lowlevel.h>
#include <sys/mount.h>
#include <ext2fs/ext2fs.h>
#include <sys/signal.h>

/************************* BEGIN CONFIGURATION **************************/

// Uncomment the lines below to get different levels of verbosity.
// Note that ULTRAQUIET will prevent AppFS from showing *any* messages
// what-so-ever, including errors.
//
// #define DEBUGGING
// #define ULTRAQUIET

// Uncomment the line below to switch between AppFS and AppMount
// functionality.  The make command will produce two executables, one
// called appfs (the default) and one called appmount (with the parameter
// below defined).  You should only uncomment this parameter if you are
// building manually or you do not have make installed.
//
// WARNING:  If you leave this parameter defined, and you call make, you
//           will get two copies of AppMount (as appfs and appmount) and not
//           the intended AppFS file.
// #define APPMOUNT

/************************** END CONFIGURATION ***************************/

#ifdef ULTRAQUIET
#undef printf
#define printf(a...) do { } while(0);
#endif

// Quite possibly one of the worst abuses of the C++ macro system ever.
#ifdef APPMOUNT
#define appfs_debugw(args...) { \
	appfs_msg("appmount: ", args); \
}
#define appfs_debugo(args...) { \
	appfs_msg("          ", args); \
}
#else
#define appfs_debugw(args...) { \
	appfs_msg("appfs: ", args); \
}
#define appfs_debugo(args...) { \
	appfs_msg("       ", args); \
}
#endif

static inline void appfs_msg(const char* appname, const char* fmt, ...)
{
	va_list args;
	printf("%s", appname);
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

// Externally define extstart (located in extstart.c)
extern "C" {
	extern int extstart (char* disk_image, char* mount_point, int is_appfs, void (*appfs_ce_func)(void));
}

// Actually define our global variable (global.h just
// declares them extern).
bool global_is_readonly = false;
std::string global_disk_image = "";
std::string global_mount_point = "";
int global_argc = 0;
char **global_argv = NULL;

// This struct holds data that we pass to our thread
// that runs FUSE.
struct appfs_fuse_info
{
	char* disk_image;
	char* mount_point;
	int argc;
	char **argv;
};

// Predefine thread function.
void* appfs_execution_thread(void* ptr);

// Predefine execution continuation function.
extern "C"
{
	void appfs_continue_execution();
}

// This is our entry point where we call the
// fuse_main() function which handles FUSE.
#define LEADING_AREA 1 * 1024 * 1024
#define COPY_BLOCK_SIZE 1 * 1024 * 1024
int main(int argc, char *argv[])
{
	const char * disk_path = NULL;
	const char * mount_path = NULL;

#ifdef APPMOUNT
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
                appfs_debugw("error: insufficient memory\n");
                return 1;
        }

        // Now parse the arguments.
        int nerrors = arg_parse(argc, argv, argtable);

        // Check to see if there were errors.
        if (nerrors > 0 && show_help->count == 0)
        {
                printf("Usage: PROGRAM_NAME");
                arg_print_syntax(stdout, argtable, "\n");

                arg_print_errors(stdout, end, "appfs");
                return 1;
        }

        // Check to see if the user requested showing the help
        // message.
        if (show_help->count == 1)
        {
                printf("Usage: PROGRAM_NAME");
                arg_print_syntax(stdout, argtable, "\n");

                printf("AppFS - An application storage filesystem.\n\n");
                arg_print_glossary(stdout, argtable, "    %-25s %s\n");
                return 0;
        }

	// Override the disk path and mount path with those the user
	// has provided.
	disk_path = disk_image->filename[0];
	mount_path = mount_point->filename[0];
#else
	// AppFS needs a temporary location for the mountpoint.
	char mount_path_template[] = "/tmp/appfs_mount.XXXXXX";
	mount_path = mkdtemp(mount_path_template);
	if (mount_path == NULL)
	{
		appfs_debugw("error: unable to create temporary directory for mounting\n");
		return 1;
	}
#endif

	// Extract & copy our data to a temporary directory.  It woudl be really
	// nice if we could just give fuse-ext2 an offset and work with the file
	// embedded in the program, but that could require some serious rewriting.
	if (argc > 0)
	{
		// Open the file the application is running from.
		FILE * self_file;
#ifndef APPMOUNT
		self_file = fopen(argv[0], "r");
#else
		self_file = fopen(disk_path, "r");
#endif
		if (self_file != NULL)
		{
			// Obtain file size.
			fseek(self_file, 0, SEEK_END);
			long self_file_size = ftell(self_file);
			rewind(self_file);

			// Seek to 1MB in the file.
			fseek(self_file, LEADING_AREA, SEEK_SET);

			// Now open a temporary file.
			char temp_disk_path[] = "/tmp/appfs_disk.XXXXXX";
			int temp_file;
			temp_file = mkstemp(temp_disk_path);
			
			if (temp_file == -1)
			{
				// Error
				appfs_debugw("error: unable to unload package (create error)\n");
				fclose(self_file);
				return 1;
			}

			// Copy file in 1MB chunks until bytes_left is 0.
			char * buffer = (char*) malloc(sizeof(char)*COPY_BLOCK_SIZE);
			while (feof(self_file) == 0)
			{
				// Read
				size_t read_result = fread(buffer, 1, COPY_BLOCK_SIZE, self_file);
				if (read_result != COPY_BLOCK_SIZE)
				{
					if (ferror(self_file) != 0)
					{
						// Error reading file data.
						appfs_debugw("error: unable to unload package (read error)\n");
						fclose(self_file);
						close(temp_file);
						return 1;
					}
				}
				
				// Write
				ssize_t write_result = write(temp_file, buffer, read_result);
				//printf("appfs: %i\n", read_result);
				//printf("appfs: %i\n", write_result);
				if (write_result != read_result)
				{
					appfs_debugw("error: unable to unload package (write error)\n");
					fclose(self_file);
					close(temp_file);
					return 1;
				}
			}
			
			// Now close both of the files.
			fclose(self_file);
			close(temp_file);

			// ... and set the disk_path variable.
			disk_path = temp_disk_path;
		}
		else
		{
			appfs_debugw("error: unable to locate the current application file\n");
			return 1;
		}
	}
	else
	{
		appfs_debugw("error: invalid number of arguments (unable to detect current application file)\n");
		return 1;
	}

	// Convert our path names to absolute paths.
	char abspath_char[PATH_MAX+1] = "\0";
        char* abspath_ret = getcwd(abspath_char, PATH_MAX);
	if (abspath_ret == NULL)
        {
                switch (errno)
                {
                        case EACCES:
                                appfs_debugw("error: unable to access parent directory of the current working directory.\n");
                                return 1;
                        case ENOMEM:
                                appfs_debugw("error: not enough memory.\n");
                                return 1;
                        case ERANGE:
                                appfs_debugw("error: the absolute path name for the disk image is longer than appfs is capable of storing in memory.\n");
                                return 1;
                }
        }
	if (disk_path[0] != '/')
	{
		global_disk_image += abspath_char;
		global_disk_image += "/";
		global_disk_image += disk_path;
		disk_path = global_disk_image.c_str();
	}
	if (mount_path[0] != '/')
        {
                global_mount_point = abspath_char;
		global_mount_point += "/";
                global_mount_point += mount_path;
                mount_path = global_mount_point.c_str();
        }

	// Translate the mount point for FUSE and store the global
	// variables.
#ifdef APPMOUNT
	global_is_readonly = (bool)is_readonly->count;
#else
	global_is_readonly = false;
#endif
	global_disk_image = disk_path;
	global_mount_point = mount_path;
	global_argc = argc;
	global_argv = argv;

	// Now call the FUSE Ext2 system.
#ifdef DEBUGGING
	appfs_debugw("initalizing fuse-ext2...\n");
	appfs_debugw("disk image: %s\n", global_disk_image.c_str());
	appfs_debugw("mount point: %s\n", global_mount_point.c_str());
	appfs_debugw("superblock start: %i\n", SUPERBLOCK_OFFSET);
#endif
#ifdef APPMOUNT
	appfs_debugw("The filesystem will now be mounted.  In order to write the changes\n");
        appfs_debugo("back to the application image, you need to unmount the image.  When ready:\n");
        appfs_debugo("  * Run 'umount %s'", global_mount_point.c_str());
        printf("\n");
	int ret = extstart((char*)global_disk_image.c_str(), (char*)global_mount_point.c_str(), false, appfs_continue_execution);
#else
	int ret = extstart((char*)global_disk_image.c_str(), (char*)global_mount_point.c_str(), true, appfs_continue_execution);
#endif

	// At this point the filesystem has closed.  We need to reattach the temporary disk image
	// onto the main executable to preserve written changes.  First we're going to remove the
	// temporary mount point (if AppFS mode).
#ifndef APPMOUNT
	if (rmdir((char*)global_mount_point.c_str()) != 0)
	{
		appfs_debugw("error: unable to remove temporary mount directory\n");
		return 1;
	}
#endif

	// Open the file the application is running from.
	FILE * self_file;
	self_file = fopen(argv[0], "r");
	if (self_file != NULL)
	{
		// Obtain file size.
		fseek(self_file, 0, SEEK_END);
		long self_file_size = ftell(self_file);
		rewind(self_file);

		// Now open the disk file.
		FILE * temp_file = fopen((char*)global_disk_image.c_str(), "r");
			
		if (temp_file == NULL)
		{
			// Error
			appfs_debugw("error: unable to save package (unable to open disk file)\n");
			fclose(self_file);
			return 1;
		}

		// Now get a temporary file to hold the new application.
		char new_disk_path[] = "/tmp/appfs_save.XXXXXX";
		int new_file = mkstemp(new_disk_path);

		if (new_file == -1)
                {
                        // Error
                        appfs_debugw("error: unable to save package (unable to open save file)\n");
                        fclose(self_file);
                        return 1;
                }

		// Copy the first 1MB from the self_file to the new file.
		char * buffer = (char*) malloc(sizeof(char)*COPY_BLOCK_SIZE);
		size_t nread_result = fread(buffer, 1, LEADING_AREA, self_file);
		if (nread_result != COPY_BLOCK_SIZE)
		{
			if (ferror(self_file) != 0)
			{
				appfs_debugw("error: unable to save package (read error, application section)\n");
                                fclose(self_file);
                                fclose(temp_file);
				close(new_file);
                                return 1;
			}
		}

		ssize_t nwrite_result = write(new_file, buffer, nread_result);
                if (nwrite_result != nread_result)
                {
                	appfs_debugw("error: unable to save package (write error, application section)\n");
                        fclose(self_file);
                        fclose(temp_file);
			close(new_file);
                        return 1;
		}

		// Copy file in 1MB chunks until bytes_left is 0.
		while (feof(temp_file) == 0)
		{
			// Read
			size_t read_result = fread(buffer, 1, COPY_BLOCK_SIZE, temp_file);
			if (read_result != COPY_BLOCK_SIZE)
			{
				if (ferror(self_file) != 0)
				{
					// Error reading file data.
					appfs_debugw("error: unable to save package (read error, fs section)\n");
					fclose(self_file);
					fclose(temp_file);
					close(new_file);
					return 1;
				}
			}
				
			// Write
			size_t write_result = write(new_file, buffer, read_result);
			//printf("appfs: %i\n", read_result);
			//printf("appfs: %i\n", write_result);
			if (write_result != read_result)
			{
				appfs_debugw("error: unable to save package (write error, section)\n");
				fclose(self_file);
				fclose(temp_file);
				close(new_file);
				return 1;
			}
		}
			
		// Now close both of the files.
		fclose(self_file);
		fclose(temp_file);
		close(new_file);

		// Copy the new file in-place.
		char oldtmp_path[] = "/tmp/appfs_cleanup.XXXXXX";
		if (mkdtemp(oldtmp_path) == NULL)
		{
			appfs_debugw("error: unable to save package (create temporary directory error)\n");
			return 1;
		}
		std::string t = oldtmp_path;
		t = t + "/appmemory";
		if (rename(argv[0], t.c_str()) != 0)
		{
			appfs_debugw("error: unable to save package (rename error, old -> oldtmp)\n");
			return 1;
		}
		if (rename(new_disk_path, argv[0]) != 0)
		{
			appfs_debugw("error: unable to save package (rename error, new -> old)\n");
			return 1;
		}
		if (remove(t.c_str()) != 0)
		{
			appfs_debugw("error: unable to save package (delete error, oldtmp -> *)\n");
			return 1;
		}
		if (rmdir(oldtmp_path) != 0)
                {
                        appfs_debugw("error: unable to save package (delete temporary directory error)\n");
                        return 1;
                }

		// Set the permissions on the new file.
		// TODO: Really should read the old permissions and then assign them
		//       to the new file...
		if (chmod(argv[0], S_IRUSR | S_IWUSR | S_IXUSR) != 0)
		{
			appfs_debugw("error: unable to save package (set permissions error)\n");
			return 1;
		}
	}
	else
	{
		appfs_debugw("error: unable to save package (unable to open application file)\n");
		return 1;
	}

	// Remove the temporary file.
	remove(global_disk_image.c_str());

	// Return
	return ret;
}

extern "C"
{
	void appfs_continue_execution()
	{
		// Start the thread.
	        pthread_t appfs_fuse_thread;
	        appfs_fuse_info * appfs_fuse_thread_info = new appfs_fuse_info();
	        appfs_fuse_thread_info->disk_image = (char*)global_disk_image.c_str();
	        appfs_fuse_thread_info->mount_point = (char*)global_mount_point.c_str();
		appfs_fuse_thread_info->argc = global_argc;
		appfs_fuse_thread_info->argv = global_argv;
	        int iret = pthread_create(&appfs_fuse_thread, NULL, appfs_execution_thread, (void*) appfs_fuse_thread_info);
	        // Caution: The thread will delete appfs_fuse_thread_info from memory!
		//printf("appfs: exiting appfs_continue_execution()...\n");
	}
}

bool file_exists(const char* filename)
{
	if (FILE * file = fopen(filename, "r"))
	{
		fclose(file);
		return true;
	}
	return false;
}

void* appfs_execution_thread(void* ptr)
{
	appfs_fuse_info * appfs_fuse_thread_info = (appfs_fuse_info*) ptr;
	std::string disk_image = appfs_fuse_thread_info->disk_image;
	std::string mount_point = appfs_fuse_thread_info->mount_point;
	int argc = appfs_fuse_thread_info->argc;
	char **argv = appfs_fuse_thread_info->argv;
	delete appfs_fuse_thread_info;

	// This function is where we perform our application execution
	// and all that (since we must leave FUSE maintaining the file
	// system in the background).
#ifdef DEBUGGING
	appfs_debugw("fuse-ext2 started!\n");
#endif

#ifndef APPMOUNT
	std::string entry_point_path = mount_point + "/EntryPoint";
	if (file_exists(entry_point_path.c_str()))
	{
		// We're going to start the application and wait for execution
		// to finish.
		std::string command = entry_point_path.c_str();
		for (int i = 1; i < argc; i += 1)
                {
			// There must be a better way of replacing characters to
			// ensure the arguments are passed exactly.
			std::string insane_argv = argv[i];
			std::string sane_argv = "";
			for (int a = 0; a < insane_argv.length(); a += 1)
			{
				if (insane_argv[a] == '\\')
					sane_argv.append("\\\\");
				else if (insane_argv[a] == '"')
					sane_argv.append("\\\"");
				else
					sane_argv += insane_argv[a];
			}
			command = command + " \"" + sane_argv + "\"";
                }

		char * old_cwd = getcwd(NULL, 0);
		chdir(mount_point.c_str());
		system(command.c_str());
		chdir(old_cwd);

#ifdef DEBUGGING		
		appfs_debugw("closing mountpoint.\n");
#endif
		kill(getpid(), SIGHUP);
		return NULL;
	}
	else
	{
		// The entry point does not exist.  Notify the user that
		// no application will be run (and don't terminate the
		// parent pid before returning from this function).
		appfs_debugw("The application does not have an entry point.  The filesystem will be");
		appfs_debugo("mounted in the foreground.  In order to stop the process:");
		appfs_debugo(" * Hit Ctrl-C or\n");
		appfs_debugo(" * Run 'umount %s'", mount_point.c_str());
		printf("\n");
		return NULL;
	}
#endif
}
