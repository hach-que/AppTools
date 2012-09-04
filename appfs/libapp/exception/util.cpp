/* vim: set ts=4 sw=4 tw=0 :*/

#include <libapp/exception/util.h>

namespace AppLib
{
    namespace Exception
    {
        const char* InvalidOpenMode::what() const throw()
        {
            return "The open mode must be either \"r\", \"w\" or \"rw\".";
        }
    }
}

