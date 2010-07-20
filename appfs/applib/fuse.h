/*

Header file for FuseLink.

This class recieves callbacks from FUSE and handles each operation
by passing them to the actual FS class in a C++ friendly way.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                22nd June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef CLASS_FUSELINK
#define CLASS_FUSELINK

#include "fs.h"

namespace AppLib
{
	namespace FUSE
	{
		class FuseLink
		{
			public:
				static LowLevel::FS filesystem;
				static int getattr(const char* path, struct stat *stbuf);
				// TODO: Implement other FUSE callbacks here
		};

		class Mounter
		{
			public:
				Mounter(const char* disk_image, char const* mount_path,
						bool foreground, void (*continue_func)(void));
		}
	}
}
#endif