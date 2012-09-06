/* vim: set ts=4 sw=4 tw=0 :*/

#ifndef CLASS_FS
#define CLASS_FS

#include <libapp/config.h>

#include <string>
#include <cstdio>
#include <libapp/fsfile.h>
#include <libapp/lowlevel/blockstream.h>
#include <libapp/lowlevel/fs.h>
#include <libapp/exception/package.h>
#include <libapp/exception/fs.h>
#include <libapp/exception/util.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace AppLib
{
    class FS
    {
    private:
        AppLib::LowLevel::BlockStream * stream;
        AppLib::LowLevel::FS * filesystem;

    public:
        FS(std::string packagePath);
        void getattr(std::string path, struct stat& stbufOut)
            const throw(Exception::PathNotValid, Exception::FileNotFound,
                    Exception::InternalInconsistency);
        std::string readlink(std::string path)
            const throw(Exception::PathNotValid, Exception::FileNotFound,
                    Exception::NotSupported, Exception::InternalInconsistency);
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
        int statfs(std::string path, struct statvfs& stvfsOut);

    private:
        bool checkPathExists(std::string path) const throw();
        bool checkPathIsValid(std::string path) const throw();
        bool checkPathRenamability(std::string path, uid_t uid) const throw();
        bool checkPermission(std::string path, char op, uid_t uid, gid_t gid) const throw();
        bool retrievePathToINode(std::string path, LowLevel::INode& out, int limit = 0) const throw();
        bool retrieveParentPathToINode(std::string path, LowLevel::INode& out) const throw();
        void saveINode(LowLevel::INode& buf)
            throw (Exception::INodeSaveInvalid, Exception::INodeSaveFailed);
        void saveNewINode(uint32_t pos, LowLevel::INode& buf)
            throw (Exception::INodeSaveInvalid, Exception::INodeSaveFailed);
        int extractMaskFromMode(mode_t mode) const throw();
        std::string extractBasenameFromPath(std::string path) const throw();
        LowLevel::INode assignNewINode(LowLevel::INodeType::INodeType type, uint32_t& posOut)
            throw (Exception::INodeSaveInvalid, Exception::INodeSaveFailed,
                    Exception::NoFreeSpace, Exception::INodeExhaustion);
    };
}

#endif
