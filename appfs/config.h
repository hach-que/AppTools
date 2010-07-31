/*

Header file for config.h

This file is the global include file for AppFS / AppMount.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#define FUSE_USE_VERSION 26

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <argtable2.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <fuse.h>
#include <fuse/fuse_lowlevel.h>
#include <sys/mount.h>
#include <sys/signal.h>

/************************* BEGIN CONFIGURATION **************************/

// Uncomment the lines below to get different levels of verbosity.
// Note that ULTRAQUIET will prevent AppFS from showing *any* messages
// what-so-ever, including errors.
//
// #define DEBUGGING 1
// #define ULTRAQUIET 1

// Uncomment the line below to switch between AppFS and AppMount
// functionality.  The make command will produce two executables, one
// called appfs (the default) and one called appmount (with the parameter
// below defined).  You should only uncomment this parameter if you are
// building manually or you do not have make installed.
//
// WARNING:  If you leave this parameter defined, and you call make, you
//           will get two copies of AppMount (as appfs and appmount) and not
//           the intended AppFS file.
// #define APPMOUNT 1

/************************** END CONFIGURATION ***************************/
