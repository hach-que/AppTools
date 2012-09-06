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

        const char* NoFreeSpace::what() const throw()
        {
            return "There was no free space on the containing medium with which to expand the package.";
        }

        const char* AccessDenied::what() const throw()
        {
            return "Access was denied to the specified file or directory.";
        }

        const char* PathNotValid::what() const throw()
        {
            return "The specified path was not in the correct format.";
        }
    }
}

