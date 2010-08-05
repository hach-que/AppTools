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
			this->state = std::ios::goodbit;

#ifdef WIN32
			this->fd = _open(filename.c_str(), O_RDWR | O_BINARY);
#else
			this->fd = _open(filename.c_str(), O_RDWR);
#endif
			if (this->fd == -1)
			{
				Logging::showErrorW("Unable to open specified file as BlockStream.");
				this->invalid = true;
				this->opened = false;
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}
			else
			{
				this->invalid = false;
				this->opened = true;
			}
		}

		void BlockStream::write(const char * data, std::streamsize count)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			int res = _write(this->fd, data, (unsigned int)count);
			if (res == -1)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}
#ifndef WIN32
			else
				this->fpos += res;
#endif

			return;
		}

		std::streamsize BlockStream::read(char * out, std::streamsize count)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

			int res = _read(this->fd, out, count);
			if (res == -1)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}
			else if (res < count)
			{
				this->clear(std::ios::eofbit | std::ios::failbit);
#ifndef WIN32
				this->fpos += res;
#endif
				return res;
			}
			else
			{
#ifndef WIN32
				this->fpos += res;
#endif
				return res;
			}
		}
		
		void BlockStream::close()
		{
			_close(this->fd);
			this->opened = false;
		}

		void BlockStream::seekp(std::streampos pos, int seekMode)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			int res = _lseek(this->fd, pos, seekMode);
			if (res == -1)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
			}
		}

		void BlockStream::seekg(std::streampos pos, int seekMode)
		{
			if (this->invalid || !this->opened || this->fail())
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			int res = _lseek(this->fd, pos, seekMode);
			if (res == -1)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
			}
		}

		std::streampos BlockStream::tellp()
		{
			if (this->invalid || !this->opened || this->fail())
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

#ifdef WIN32
			long res = _tell(this->fd);
			if (res == -1)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}
			else
				return res;
#else
			return this->fpos;
#endif
		}

		std::streampos BlockStream::tellg()
		{
			if (this->invalid || !this->opened || this->fail())
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

#ifdef WIN32
			long res = _tell(this->fd);
			if (res == -1)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}
			else
				return res;
#else
			return this->fpos;
#endif
		}

		bool BlockStream::is_open()
		{
			return this->opened;
		}

		std::ios::iostate BlockStream::rdstate()
		{
			return this->state;
		}

		void BlockStream::clear()
		{
			this->state = std::ios::goodbit;
		}

		void BlockStream::clear(std::ios::iostate state)
		{
			this->state = state;
		}

		bool BlockStream::good()
		{
			return (this->state == std::ios::goodbit);
		}

		bool BlockStream::bad()
		{
			return (this->state & std::ios::badbit != 0);
		}

		bool BlockStream::eof()
		{
			return (this->state & std::ios::eofbit != 0);
		}

		bool BlockStream::fail()
		{
			return (this->state & std::ios::failbit != 0);
		}
	}
}

