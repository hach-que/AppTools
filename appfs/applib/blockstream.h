/*

Header file for BlockStream.

This class is capable of caching write and read requests in memory
on a per-block basis and also allows AppLib to read and write to
the current executable file (unlike std::fstream).

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                5th August 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef CLASS_BLOCKSTREAM
#define CLASS_BLOCKSTREAM

#include "config.h"

#include <string>
#include <iostream>
#include "logging.h"
#include "endian.h"
#include <errno.h>

namespace AppLib
{
	namespace LowLevel
	{
		class BlockStream
		{
			public:
				BlockStream(std::string filename);
				void write(const char * data, std::streamsize count);
				std::streamsize read(char * out, std::streamsize count);
				void close();
				void seekp(std::streampos pos, int seekMode = SEEK_SET);
				void seekg(std::streampos pos, int seekMode = SEEK_SET);
				std::streampos tellp();
				std::streampos tellg();

				// State functions.
				bool is_open();
				std::ios::iostate rdstate();
				void clear();
				void clear(std::ios::iostate state);
				bool good();
				bool bad();
				bool eof();
				bool fail();

			private:
				int fd;
				bool opened;
				bool invalid;
#ifndef WIN32
				uint32_t fpos;
#endif
				std::ios::iostate state;
		};
	}
}

#endif

