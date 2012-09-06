/* vim: set ts=4 sw=4 tw=0 et ai :*/

#ifndef CLASS_FUSELINK
#define CLASS_FUSELINK

#include <libapp/config.h>

#include <fuse.h>
#include <stdio.h>
#include <errno.h>
#include <libapp/lowlevel/fs.h>

namespace AppLib
{
    namespace FUSE
    {
        class FuseLink
        {
        public:
            static LowLevel::FS * filesystem;
            static void (*continuefunc) (void);
            static int getattr(const char *path, struct stat *stbuf);
            static int readlink(const char *path, char *out, size_t size);
            static int mknod(const char *path, mode_t mask, dev_t devid);
            static int mkdir(const char *path, mode_t mask);
            static int unlink(const char *path);
            static int rmdir(const char *path);
            static int symlink(const char *path, const char *source);
            static int rename(const char *path, const char *dest);
            static int link(const char *path, const char *source);
            static int chmod(const char *path, mode_t mode);
            static int chown(const char *path, uid_t user, gid_t group);
            static int truncate(const char *path, off_t size);
            static int open(const char *path, struct fuse_file_info *options);
            static int read(const char *path, char *out, size_t length,
                            off_t offset, struct fuse_file_info *options);
            static int write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
            static int statfs(const char *, struct statvfs *);
            static int flush(const char *, struct fuse_file_info *);
            static int release(const char *, struct fuse_file_info *);
            static int fsync(const char *, int, struct fuse_file_info *);
            static int setxattr(const char *, const char *, const char *, size_t, int);
            static int getxattr(const char *, const char *, char *, size_t);
            static int listxattr(const char *, char *, size_t);
            static int removexattr(const char *, const char *);
            static int opendir(const char *, struct fuse_file_info *);
            static int readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
            static int releasedir(const char *, struct fuse_file_info *);
            static int fsyncdir(const char *, int, struct fuse_file_info *);
            static void *init(struct fuse_conn_info *conn);
            static void destroy(void *);
            static int access(const char *, int);
            static int create(const char *, mode_t, struct fuse_file_info *);
            static int ftruncate(const char *, off_t, struct fuse_file_info *);
            static int fgetattr(const char *, struct stat *, struct fuse_file_info *);
            static int lock(const char *, struct fuse_file_info *, int cmd, struct flock *);
            static int utimens(const char *, const struct timespec tv[2]);
            static int bmap(const char *, size_t blocksize, uint64_t * idx);
            static int ioctl(const char *, int cmd, void *arg,
                             struct fuse_file_info *, unsigned int flags, void *data);
            static int poll(const char *, struct fuse_file_info *,
                            struct fuse_pollhandle *ph, unsigned *reventsp);
        };

        class Mounter
        {
        public:
            Mounter(const char *disk_image, char const *mount_point,
                    bool foreground, bool allow_other, void (*continue_func) (void));
            int getResult();

        private:
            int mountResult;
        };

        class API
        {
        public:
            static void load(const char* disk_image);
            static void unload();
        };
        
        class Macros
        {
        public:
            static int checkPathExists(const char *path);
            static int checkPathNotExists(const char *path);
            static int checkPathIsValid(const char *path);
            static int checkPathRenamability(const char *path);
            static int checkPermission(const char *path, char op, int uid, int gid);
            static int retrievePathToINode(const char *path, LowLevel::INode * out);
            static int retrieveParentPathToINode(const char *path, LowLevel::INode * out);
            static int saveINode(LowLevel::INode * buf);
            static int saveNewINode(uint32_t pos, LowLevel::INode * buf);
            static int extractMaskFromMode(mode_t mode);
            static const char *extractBasenameFromPath(const char *path);
            static int assignNewINode(LowLevel::INode * buf, LowLevel::INodeType::INodeType type,
                                      uint32_t & pos);
        };

        struct FUSEData
        {
            std::string diskImage;
            std::string mountPoint;
            AppLib::LowLevel::FS * filesystem;
            bool readonly;
        };
    }
}
#endif