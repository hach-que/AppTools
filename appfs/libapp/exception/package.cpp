/* vim: set ts=4 sw=4 tw=0 :*/

#include <libapp/exception/package.h>

namespace AppLib
{
    namespace Exception
    {
        const char* FileNotFound::what() const throw()
        {
            return "The specified file was not found.";
        }

        const char* PackageNotFound::what() const throw()
        {
            return "The specified package was not found.";
        }

        const char* PackageNotValid::what() const throw()
        {
            return "The specified package was not valid.";
        }
    }
}

