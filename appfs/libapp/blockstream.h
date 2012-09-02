/*

Header file for BlockStream.

This class is capable of caching write and read requests in memory
on a per-block basis.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                5th August 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#ifndef CLASS_BLOCKSTREAM
#define CLASS_BLOCKSTREAM

#include <libapp/config.h>

#include <string>
#include <iostream>
#include <libapp/logging.h>
#include <libapp/endian.h>
#include <errno.h>
#include <pthread.h>

namespace AppLib
{
	namespace LowLevel
	{
		class BlockStream
		{
		      public:
			BlockStream(std::string filename);
			void write(const char *data, std::streamsize count);
			 std::streamsize read(char *out, std::streamsize count);
			void close();
			void seekp(std::streampos pos, std::ios_base::seekdir dir = std::ios_base::beg);
			void seekg(std::streampos pos, std::ios_base::seekdir dir = std::ios_base::beg);
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
			 std::fstream * fd;
			bool opened;
			bool invalid;
			pthread_mutex_t * mutex;
		};
	}
}

#endif
