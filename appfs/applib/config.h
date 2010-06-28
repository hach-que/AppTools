/*

Header file for Logging.

This is the AppFS configuration file.  It specifies settings
such as offsets.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                27th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef APPFS_CONFIG
#define APPFS_CONFIG

/*********** Start Configuration *************/
// Defines the offsets and lengths used for different
// sections within AppFS packages.  You should not change
// these for compatibility reasons.
#define OFFSET_BOOTSTRAP 0
#define LENGTH_BOOTSTRAP (1024 * 1024)
#define OFFSET_LOOKUP    (1024 * 1024)
#define LENGTH_LOOKUP    (256 * 1024)
#define OFFSET_ROOTINODE (1024 * 1024 + 256 * 1024)
#define LENGTH_ROOTINODE (4376)
#define OFFSET_DATA      (1024 * 1024 + 256 * 1024 + 4376)

// Defines whether or not the test suite should automatically
// create a blank AppFS file for testing if it doesn't exist
// in the local directory.
#define TESTSUITE_AUTOCREATE 1

// Define the raw block sizes.  The directory block size must
// be a multiple of the file block size.
#define BSIZE_FILE 512
#define BSIZE_DIRECTORY 4096

// Number of subdirectories / subfiles allowed in a single
// directory.
#define DIRECTORY_CHILDREN_MAX 1903

/************ End Configuration **************/
#endif