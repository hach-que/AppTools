#ifndef CLASS_FSFILE
#define CLASS_FSFILE
#include <iostream>
#include "config.h"

namespace AppLib
{
	namespace LowLevel
	{
		class FS;

		class FSFile
		{
		public:
			FSFile(FS * filesystem, std::fstream * fd, uint16_t inodeid);
			void open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
			void write(const char * data, std::streamsize count);
			std::streamsize read(char * out, std::streamsize count);
			bool truncate(std::streamsize len);
			void close();
			void seekp(std::streampos pos);
			void seekg(std::streampos pos);
			std::streampos tellp();
			std::streampos tellg();

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
			FS * filesystem;
			std::fstream * fd;
			bool opened;
			bool invalid;
			uint32_t posp;
			uint32_t posg;
			std::ios::iostate state;
		};
	}
}

#endif
