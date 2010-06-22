/*

Code file for FuseLink.

This class recieves callbacks from FUSE and handles each operation
by passing them to the actual FS class in a C++ friendly way.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                21th June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include <fuse.h>
#include <stdio.h>
#include <errno.h>

#include <string>

public static class FuseLink
{
	public static FS filesystem = NULL;

	public static int getattr(const char* path, struct stat *stbuf)
	{
		int result = 0;
		
		// Create a new stat object in the stbuf position.
		memset(stbuf, 0, sizeof(struct stat));

		// Now retrieve the 
	} 
}
