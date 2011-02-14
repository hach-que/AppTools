/*

Code file for BlockStream.

This class is capable of caching write and read requests in memory
on a per-block basis and also allows AppLib to read and write to
the current executable file (unlike std::fstream).

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                5th August 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#include <string>
#include <iostream>
#include "logging.h"
#include "endian.h"
#include "blockstream.h"
#include <errno.h>
#ifndef WIN32
#define _open ::open
#define _tell ::tell
#define _lseek ::lseek
#define _write ::write
#define _read ::read
#define _close ::close
#else
#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace AppLib
{
	namespace LowLevel
	{
		BlockStream::BlockStream(std::string filename)
		{
			this->opened = false;
			this->invalid = false;

			this->fd = new std::fstream(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			if (!this->fd->is_open())
			{
				Logging::showErrorW("Unable to open specified file as BlockStream.");
				this->invalid = true;
				this->opened = false;
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}
			else
			{
				this->fd->exceptions(std::ifstream::badbit | std::ios::failbit | std::ios::eofbit);
				this->invalid = false;
				this->opened = true;
			}
		}

		void BlockStream::write(const char * data, std::streamsize count)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				if (!this->fail())
					this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			if (this->fd->tellp() % 4096 == 0 && count == 4)
			{
				// Display debugging.
				AppLib::Logging::showWarningW("Writing 4 bytes to start of block: %X",
					(unsigned int)data[0], (unsigned int)data[1],
					(unsigned int)data[2], (unsigned int)data[3]);
			}

			this->fd->write(data, count);
		}

		std::streamsize BlockStream::read(char * out, std::streamsize count)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				if (!this->fail())
					this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

			std::cout << "blockstream: Reading " << count << " at " << this->tellg() << "." << std::endl;
			this->fd->read(out, count);
			return this->fd->gcount();
		}
		
		void BlockStream::close()
		{
			this->fd->close();
			this->opened = false;
		}

		void BlockStream::seekp(std::streampos pos, std::ios_base::seekdir dir)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				if (!this->fail())
					this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			this->fd->seekp(pos, dir);
		}

		void BlockStream::seekg(std::streampos pos, std::ios_base::seekdir dir)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				if (!this->fail())
					this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			this->fd->seekg(pos, dir);
		}

		std::streampos BlockStream::tellp()
		{
			if (this->invalid || !this->opened || this->fail())
			{
				if (!this->fail())
					this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

			return this->fd->tellp();
		}

		std::streampos BlockStream::tellg()
		{
			if (this->invalid || !this->opened || this->fail())
			{
				if (!this->fail())
					this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

			return this->fd->tellg();
		}

		

		bool BlockStream::is_open()
		{
			return this->fd->is_open();
		}

		std::ios::iostate BlockStream::rdstate()
		{
			return this->fd->rdstate();
		}

		void BlockStream::clear()
		{
			this->fd->clear();
		}

		void BlockStream::clear(std::ios::iostate state)
		{
			this->fd->clear(state);
		}

		bool BlockStream::good()
		{
			return this->fd->good();
		}

		bool BlockStream::bad()
		{
			return this->fd->bad();
		}

		bool BlockStream::eof()
		{
			return this->fd->eof();
		}

		bool BlockStream::fail()
		{
			return this->fd->fail();
		}
	}
}

