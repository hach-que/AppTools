/*

Header file for FSFile.

This class reads and writes to files within AppFS packages, allowing
applications to modify the contained files similar to how they would
operate on files through std::fstream.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                31st July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef CLASS_FSFILE
#define CLASS_FSFILE

#include "config.h"

#include <iostream>
#include "blockstream.h"

namespace AppLib
{
	namespace LowLevel
	{
		class FS;

		class FSFile
		{
		      public:
			FSFile(FS * filesystem, BlockStream * fd, uint16_t inodeid);
			void open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
			void write(const char *data, std::streamsize count);
			 std::streamsize read(char *out, std::streamsize count);
			bool truncate(std::streamsize len);
			void close();
			void seekp(std::streampos pos);
			void seekg(std::streampos pos);
			 std::streampos tellp();
			 std::streampos tellg();
			uint32_t size();

			// State functions.
			 std::ios::iostate rdstate();
			void clear();
			void clear(std::ios::iostate state);
			bool good();
			bool bad();
			bool eof();
			bool fail();

		      private:
			 uint16_t inodeid;
			FS *filesystem;
			BlockStream *fd;
			bool opened;
			bool invalid;
			uint32_t posp;
			uint32_t posg;
			 std::ios::iostate state;
		};
	}
}

#endif
