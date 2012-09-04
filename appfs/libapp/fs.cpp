/* vim: set ts=4 sw=4 tw=0 :*/

#include <exception>
#include <libapp/fs.h>
#include <libapp/exception/package.h>

namespace AppLib
{
    FS::FS(std::string path)
    {
        this->stream = new LowLevel::BlockStream(path.c_str());
        if (!this->stream->is_open())
        {
            delete this->stream;
            throw Exception::PackageNotFound();
        }
        this->filesystem = new LowLevel::FS(this->stream);
        if (!this->filesystem->isValid())
        {
            this->stream->close();
            delete this->stream;
            delete this->filesystem;
            throw Exception::PackageNotValid();
        }
    }
}
