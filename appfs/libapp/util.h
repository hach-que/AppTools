/* vim: set ts=4 sw=4 tw=0 et ai :*/

#ifndef CLASS_UTIL
#define CLASS_UTIL

#include <libapp/config.h>
#include <string>
#include <iostream>
#include <fstream>
#include <libapp/blockstream.h>

namespace AppLib
{
    namespace LowLevel
    {
        class Util
        {
            public:
                static void seekp_ex(LowLevel::BlockStream * fd, std::streampos pos);
                static bool fileExists(std::string filename);
                static void sanitizeArguments(char ** argv, int argc, std::string & command, int start);
                static bool extractBootstrap(std::string source, std::string dest);
                static char* getProcessFilename();
                static bool createPackage(std::string path, const char* appname, const char* appver,
                            const char* appdesc, const char* appauthor);
        };
    }
}

#endif
