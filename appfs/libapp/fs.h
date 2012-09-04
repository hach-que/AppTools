/* vim: set ts=4 sw=4 tw=0 :*/

#ifndef CLASS_FS
#define CLASS_FS

#include <libapp/config.h>

#include <string>
#include <cstdio>
#include <libapp/fsfile.h>
#include <libapp/lowlevel/blockstream.h>
#include <libapp/lowlevel/fs.h>

namespace AppLib
{
    class FS
    {
    private:
        AppLib::LowLevel::BlockStream * stream;
        AppLib::LowLevel::FS * filesystem;

    public:
        FS(std::string packagePath);
        int getattr(std::string path, struct stat* stbuf);
        int readlink(std::string path, std::string& out);
        int mknod(std::string path, mode_t mask, dev_t devid);
        int mkdir(std::string path, mode_t mask);
        int unlink(std::string path);
        int rmdir(std::string path);
        int symlink(std::string linkPath, std::string targetPath);
        int rename(std::string srcPath, std::string destPath);
        int link(std::string linkPath, std::string targetPath);
        int chmod(std::string path, mode_t mask);
        int chown(std::string path, uid_t user, gid_t group);
        int truncate(std::string path, off_t size);
        FSFile* open(std::string path);
        int statfs(std::string path, struct statvfs* stbuf);
    };
}

#endif
