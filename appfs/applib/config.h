/*

Header file for Logging.

This is the AppFS configuration file.  It specifies settings
such as offsets.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                27th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#define FUSE_USE_VERSION 26

#ifndef APPFS_CONFIG
#define APPFS_CONFIG

/*********** Start Configuration *************/
// Defines the offsets and lengths used for different
// sections within AppFS packages.  You should not change
// these for compatibility reasons.
#define OFFSET_BOOTSTRAP 0
#define LENGTH_BOOTSTRAP (3 * 1024 * 1024)
#define OFFSET_LOOKUP    LENGTH_BOOTSTRAP
#define LENGTH_LOOKUP    (256 * 1024)
#define OFFSET_FSINFO    (LENGTH_BOOTSTRAP + LENGTH_LOOKUP)
#define LENGTH_FSINFO    (4096)
#define OFFSET_DATA      (LENGTH_BOOTSTRAP + LENGTH_LOOKUP + LENGTH_FSINFO)

// Name of the filesystem implementation.  Must be 9 characters
// because the automatic terminating NULL character makes it 10
// in total (and we write out 10 bytes to our FSINFO block).
#define FS_NAME "AppFS\0\0\0\0"

// Defines whether or not the test suite should automatically
// create a blank AppFS file for testing if it doesn't exist
// in the local directory.
#define TESTSUITE_AUTOCREATE 1

// Define the raw block sizes.  The directory block size must
// be a multiple of the file block size.
#define BSIZE_FILE      4096
#define BSIZE_DIRECTORY 4096

// Number of subdirectories / subfiles allowed in a single
// directory.
#define DIRECTORY_CHILDREN_MAX 1901

// Define the sizes of each of the header types.
#define HSIZE_FILE       306
#define HSIZE_SEGINFO    8
#define HSIZE_FREELIST   8
#define HSIZE_FSINFO     1614
#define HSIZE_DIRECTORY  294

/************ End Configuration **************/

#define LIBRARY_VERSION_MAJOR 0
#define LIBRARY_VERSION_MINOR 1
#define LIBRARY_VERSION_REVISION 0

#if defined(_MSC_VER)
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef unsigned int mode_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned int pid_t;
#define ENOTSUP 95
#define EALREADY 114
#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#endif

#endif
