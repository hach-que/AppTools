#include "fsfile.h"
#include "fs.h"
#include "util.h"
#include "logging.h"
#include "blockstream.h"
#include <map>
#include <math.h>

namespace AppLib
{
	namespace LowLevel
	{
		FSFile::FSFile(FS * filesystem, BlockStream * fd, uint16_t inodeid)
		{
			this->inodeid = inodeid;
			this->filesystem = filesystem;
			this->fd = fd;
			this->opened = false;
			this->invalid = false;
			this->posg = 0;
			this->posp = 0;
			this->state = std::ios::goodbit;
		}

		void FSFile::open(std::ios_base::openmode mode)
		{
			INode node = this->filesystem->getINodeByID(this->inodeid);
			this->invalid = (node.type != INodeType::INT_FILEINFO);
			if (this->invalid)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			// TODO: Check permissions of current user / group
			//       to see whether it can be opened.  If it fails
			//       to open, set the badbit on.
			this->opened = true;

			if (false /* <failed> */)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
			}
		}

		void FSFile::write(const char * data, std::streamsize count)
		{
			// TODO: Must be reimplemented under New File Storage system.
			return;
		}

		std::streamsize FSFile::read(char * out, std::streamsize count)
		{
			// TODO: Must be reimplemented under New File Storage system.
			return 0;
		}

		bool FSFile::truncate(std::streamsize len)
		{
			FSResult::FSResult fres = this->filesystem->truncateFile(this->inodeid, len);
			return (fres == FSResult::E_SUCCESS);
		}

		void FSFile::close()
		{
			this->opened = false;
		}

		void FSFile::seekp(std::streampos pos)
		{
			if (this->invalid || !this->opened)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			this->posp = pos;
		}

		void FSFile::seekg(std::streampos pos)
		{
			if (this->invalid || !this->opened)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}
			
			this->posg = pos;
		}

		std::streampos FSFile::tellp()
		{
			return this->posp;
		}

		std::streampos FSFile::tellg()
		{
			return this->posg;
		}

		std::ios::iostate FSFile::rdstate()
		{
			return this->state;
		}

		void FSFile::clear()
		{
			this->state = std::ios::goodbit;
		}

		void FSFile::clear(std::ios::iostate state)
		{
			this->state = state;
		}

		bool FSFile::good()
		{
			return (this->state == std::ios::goodbit);
		}

		bool FSFile::bad()
		{
			return (this->state & std::ios::badbit != 0);
		}

		bool FSFile::eof()
		{
			return (this->state & std::ios::eofbit != 0);
		}

		bool FSFile::fail()
		{
			return (this->state & std::ios::failbit != 0);
		}
	}
}
