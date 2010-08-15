/*

Code file for Util.

This class contains low-level utility functions which do not
suit any other existing class.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                28th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#include <string>
#include <iostream>
#include <fstream>
#include "util.h"
#include "logging.h"
#include "blockstream.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

namespace AppLib
{
	namespace LowLevel
	{
		void Util::seekp_ex(LowLevel::BlockStream * fd, std::streampos pos)
		{
			fd->clear();
			fd->seekp(pos);
			if (fd->fail())
			{
				fd->clear();
				// This means that pos is beyond the current filesize.
				// We need to expand the file by writing NUL characters
				// until fd->tellp is equal to pos.
				fd->seekp(0, std::ios::end);
				if (fd->fail())
				{
					Logging::showErrorW("Unable to expand the provided file desciptor to the desired position.");
				}
				char zero = 0;
				while (fd->tellp() != pos)
				{
					fd->write(&zero, 1);
				}
			}
		}

		bool Util::fileExists(std::string filename)
		{
			struct stat file_info;
			return (stat(filename.c_str(), &file_info) == 0);
		}
		
		void Util::sanitizeArguments(char ** argv, int argc, std::string & command, int start)
		{
			for (int i = start; i < argc; i += 1)
            {
                // There must be a better way of replacing characters to
                // ensure the arguments are passed correctly.
                std::string insane_argv = argv[i];
                std::string sane_argv = "";
                for (int a = 0; a < insane_argv.length(); a += 1)
                {
                    if (insane_argv[a] == '\\')
                        sane_argv.append("\\\\");
                    else if (insane_argv[a] == '"')
						sane_argv.append("\\\"");
                    else
                        sane_argv += insane_argv[a];
                }
                command = command + " \"" + sane_argv + "\"";
            }
		}

        bool Util::extractBootstrap(std::string source, std::string dest)
		{
			FILE * fsrc = fopen(source.c_str(), "r");
			if (fsrc == NULL) return false;
			FILE * fdst = fopen(dest.c_str(), "w");
			if (fdst == NULL) return false;
			char * buffer = (char*)malloc(1024*1024);
			size_t readres = fread(buffer, 1, 1024*1024, fsrc);
			if (ferror(fsrc) != 0) return false;
#ifdef WIN32
			size_t writeres = fwrite(buffer, 1, 1024*1024, fdst);
#else
			ssize_t writeres = fwrite(buffer, 1, 1024*1024, fdst);
#endif
			if (ferror(fdst) != 0) return false;
			if (fclose(fsrc) != 0) return false;
			if (fclose(fdst) != 0) return false;
			return true;
		}

		const char* Util::getProcessFilename()
		{
			// This is non-portable code.  For each UNIX kernel this is
			// compiled on, a section needs to be added to handle fetching
			// the current process filename.  There are a list of ways for
			// each kernel available at:
			//  * http://stackoverflow.com/questions/933850
#ifdef WIN32
			return NULL;
#else
			char * buf = (char*)malloc(PATH_MAX+1);
			for (int i = 0; i < PATH_MAX + 1; i+=1)
				buf[i] = 0;
			int ret = readlink("/proc/self/exe", buf, PATH_MAX);
			if (ret == -1)
				return NULL;
			else
				return std::string(buf).c_str();
#endif
		}
	}
}
