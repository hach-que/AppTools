/* vim: set ts=4 sw=4 tw=0 et ai :*/

#include <libapp/config.h>
#include <libapp/internal/fuselink.h>
#include <libapp/logging.h>
#include <libapp/lowlevel/fsmacro.h>
#include <string>
#include <time.h>
#include <linux/kdev_t.h>

namespace AppLib
{
    namespace FUSE
    {
        LowLevel::FS * FuseLink::filesystem = NULL;
        void (*FuseLink::continuefunc) (void) = NULL;

         Mounter::Mounter(const char *disk_image, const char *mount_point, bool foreground, bool allow_other, void (*continuefunc) (void))
        {
#ifdef WIN32
            this->mountResult = -ENOTSUP;
#else
            this->mountResult = -EALREADY;

            // Define the fuse_operations structure.
            static fuse_operations appfs_ops;
             appfs_ops.getattr = &FuseLink::getattr;
             appfs_ops.readlink = &FuseLink::readlink;
             appfs_ops.mknod = &FuseLink::mknod;
             appfs_ops.mkdir = &FuseLink::mkdir;
             appfs_ops.unlink = &FuseLink::unlink;
             appfs_ops.rmdir = &FuseLink::rmdir;
             appfs_ops.symlink = &FuseLink::symlink;
             appfs_ops.rename = &FuseLink::rename;
             appfs_ops.link = &FuseLink::link;
             appfs_ops.chmod = &FuseLink::chmod;
             appfs_ops.chown = &FuseLink::chown;
             appfs_ops.truncate = &FuseLink::truncate;
             appfs_ops.open = &FuseLink::open;
             appfs_ops.read = &FuseLink::read;
             appfs_ops.write = &FuseLink::write;
//                      appfs_ops.statfs                = &FuseLink::statfs;
//                      appfs_ops.flush                 = &FuseLink::flush;
//                      appfs_ops.release               = &FuseLink::release;
//                      appfs_ops.fsync                 = &FuseLink::fsync;
             appfs_ops.setxattr = NULL;
             appfs_ops.getxattr = NULL;
             appfs_ops.listxattr = NULL;
             appfs_ops.removexattr = NULL;
//                      appfs_ops.opendir               = &FuseLink::opendir;
             appfs_ops.readdir = &FuseLink::readdir;
//                      appfs_ops.releasedir            = &FuseLink::releasedir;
//                      appfs_ops.fsyncdir              = &FuseLink::fsyncdir;
             appfs_ops.init = &FuseLink::init;
             appfs_ops.destroy = &FuseLink::destroy;
//                      appfs_ops.access                = &FuseLink::access;
             appfs_ops.create = &FuseLink::create;
//                      appfs_ops.ftruncate             = &FuseLink::ftruncate;
//                      appfs_ops.fgetattr              = &FuseLink::fgetattr;
             appfs_ops.lock = NULL;
             appfs_ops.utimens = &FuseLink::utimens;
             appfs_ops.bmap = NULL;

            // Attempt to open the specified disk image.
             LowLevel::BlockStream * fd = new LowLevel::BlockStream(disk_image);
            if (!fd->is_open())
            {
                Logging::showErrorW("Unable to open specified disk image.");
                this->mountResult = -EIO;
                return;
            }
            fd->seekp(0);
             fd->seekg(0);

            // Attempt to create an FS object based on the disk image.
             LowLevel::FS * filesystem = new LowLevel::FS(fd);
            if (!filesystem->isValid())
            {
                Logging::showErrorW("Unable to read the specified disk image as an AppFS filesystem.");
                this->mountResult = -EIO;
                delete fd;
                 return;
            }
            FuseLink::filesystem = filesystem;

            // Set the contination function.
            FuseLink::continuefunc = continuefunc;

            // Mounts the specified disk image at the
            // specified mount path using FUSE.
            struct fuse_args fargs = FUSE_ARGS_INIT(0, NULL);

            if (fuse_opt_add_arg(&fargs, "appfs") == -1)
            {
                Logging::showErrorW("Unable to set FUSE options.");
                fuse_opt_free_args(&fargs);
                this->mountResult = -5;
                return;
            }

            if (foreground)
            {
                if (fuse_opt_add_arg(&fargs, "-f") == -1
#ifdef DEBUG
                    || fuse_opt_add_arg(&fargs, "-d") == -1
#endif
                    )
                {
                    Logging::showErrorW("Unable to set FUSE options.");
                    fuse_opt_free_args(&fargs);
                    this->mountResult = -5;
                    return;
                }
            }

            const char *normal_opts = "default_permissions,use_ino,attr_timeout=0,entry_timeout=0";
            const char *allow_opts = "allow_other,default_permissions,use_ino,attr_timeout=0,entry_timeout=0";
            const char *opts = NULL;
            if (allow_other)
            {
                Logging::showInfoW("Allowing other users access to filesystem.");
                opts = allow_opts;
            }
            else
                opts = normal_opts;

            if (fuse_opt_add_arg(&fargs, "-s") == -1 || fuse_opt_add_arg(&fargs, "-o") || fuse_opt_add_arg(&fargs, opts) == -1 || fuse_opt_add_arg(&fargs, mount_point) == -1)
            {
                Logging::showErrorW("Unable to set FUSE options.");
                fuse_opt_free_args(&fargs);
                this->mountResult = -5;
                return;
            }

            FUSEData appfs_status;
            appfs_status.filesystem = filesystem;
            appfs_status.readonly = false;
            appfs_status.mountPoint = mount_point;
            appfs_status.diskImage = disk_image;

            this->mountResult = fuse_main(fargs.argc, fargs.argv, &appfs_ops, &appfs_status);
#endif
        }

        int Mounter::getResult()
        {
            return this->mountResult;
        }
        
        void API::load(const char* disk_image)
        {
            // Attempt to open the specified disk image.
            LowLevel::BlockStream * fd = new LowLevel::BlockStream(disk_image);
            if (!fd->is_open())
            {
                Logging::showErrorW("Unable to open specified disk image.");
                return;
            }
            fd->seekp(0);
            fd->seekg(0);

            // Attempt to create an FS object based on the disk image.
            LowLevel::FS * filesystem = new LowLevel::FS(fd);
            if (!filesystem->isValid())
            {
                Logging::showErrorW("Unable to read the specified disk image as an AppFS filesystem.");
                delete fd;
                return;
            }
            FuseLink::filesystem = filesystem;
        }
        
        void API::unload()
        {
            if (FuseLink::filesystem != NULL)
            {
                delete FuseLink::filesystem;
                FuseLink::filesystem = NULL;
            }
        }

        int FuseLink::getattr(const char *path, struct stat *stbuf)
        {
            APPFS_CHECK_PATH_VALIDITY();

            int result = 0;

            // Create a new stat object in the stbuf position.
            memset(stbuf, 0, sizeof(struct stat));

            std::vector < std::string > ret = FuseLink::filesystem->splitPathBySeperators(path);
            LowLevel::INode buf = FuseLink::filesystem->getINodeByID(0);
            for (unsigned int i = 0; i < ret.size(); i += 1)
            {
                buf = FuseLink::filesystem->getChildOfDirectory(buf.inodeid, ret[i].c_str());
                if (buf.type != LowLevel::INodeType::INT_DIRECTORY && i != ret.size() - 1)
                {
                    return -ENOENT;
                }
            }
            if (buf.type != LowLevel::INodeType::INT_DIRECTORY && buf.type != LowLevel::INodeType::INT_FILEINFO && buf.type != LowLevel::INodeType::INT_SYMLINK && buf.type != LowLevel::INodeType::INT_DEVICE && buf.type != LowLevel::INodeType::INT_HARDLINK)
            {
                return -ENOENT;
            }

            // Resolve the hardlink if needed.
            if (buf.type == LowLevel::INodeType::INT_HARDLINK)
                buf = buf.resolve(FuseLink::filesystem);

            // Set the values of the stat structure.
            stbuf->st_ino = buf.inodeid;
            stbuf->st_dev = buf.dev;
            stbuf->st_mode = buf.mask;
            stbuf->st_nlink = buf.nlink;
            stbuf->st_uid = buf.uid;
            stbuf->st_gid = buf.gid;
            stbuf->st_rdev = buf.rdev;
            if (buf.type == LowLevel::INodeType::INT_FILEINFO || buf.type == LowLevel::INodeType::INT_SYMLINK || buf.type == LowLevel::INodeType::INT_DEVICE)
            {
                stbuf->st_size = buf.dat_len;
#ifndef WIN32
                stbuf->st_blksize = BSIZE_FILE;
                stbuf->st_blocks = buf.blocks;
#endif
                if (buf.type == LowLevel::INodeType::INT_FILEINFO)
                    stbuf->st_mode = S_IFREG | stbuf->st_mode;
                else if (buf.type == LowLevel::INodeType::INT_SYMLINK)
                    stbuf->st_mode = S_IFLNK | stbuf->st_mode;
            }
            else if (buf.type == LowLevel::INodeType::INT_DIRECTORY)
            {
                stbuf->st_size = BSIZE_DIRECTORY;
#ifndef WIN32
                stbuf->st_blksize = BSIZE_FILE;
                stbuf->st_blocks = BSIZE_DIRECTORY / BSIZE_FILE;
#endif
                stbuf->st_mode = S_IFDIR | stbuf->st_mode;
            }
            stbuf->st_atime = buf.atime;
            stbuf->st_mtime = buf.mtime;
            stbuf->st_ctime = buf.ctime;

            return 0;
        }

        int FuseLink::readlink(const char *path, char *out, size_t size)
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);

            if (buf.type == LowLevel::INodeType::INT_SYMLINK)
            {
                int res = FuseLink::read(path, out, size, 0, NULL);
                if (res < 0)
                    return res;
            }
            else
                return -ENOTSUP;

            return 0;
        }

        int FuseLink::mknod(const char *path, mode_t mode, dev_t devid)
        {
            APPFS_CHECK_PATH_NOT_EXISTS();
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            APPFS_ASSIGN_NEW_INODE(child, LowLevel::INodeType::INT_DEVICE);
            child.mask = Macros::extractMaskFromMode(mode);
            child.ctime = APPFS_TIME();
            child.mtime = APPFS_TIME();
            child.atime = APPFS_TIME();
            child.uid = fuse_get_context()->uid;
            child.gid = fuse_get_context()->gid;
            child.dev = MINOR(devid);
            child.rdev = MAJOR(devid);
            APPFS_BASENAME_TO_FILENAME(path, child.filename);
            APPFS_SAVE_NEW_INODE(child);

            // Now add the parent -> child relationship on disk.
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->addChildToDirectoryInode(parent.inodeid, child.inodeid);
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
            {
                Logging::showErrorW("Maximum number of children in directory reached.");
                return -EIO;
            }
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            return 0;

        }

        int FuseLink::mkdir(const char *path, mode_t mode)
        {
            APPFS_CHECK_PATH_NOT_EXISTS();
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            APPFS_ASSIGN_NEW_INODE(child, LowLevel::INodeType::INT_DIRECTORY);
            child.mask = Macros::extractMaskFromMode(mode);
            child.ctime = APPFS_TIME();
            child.mtime = APPFS_TIME();
            child.atime = APPFS_TIME();
            child.uid = fuse_get_context()->uid;
            child.gid = fuse_get_context()->gid;
            APPFS_BASENAME_TO_FILENAME(path, child.filename);
            APPFS_SAVE_NEW_INODE(child);

            // Now add the parent -> child relationship on disk.
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->addChildToDirectoryInode(parent.inodeid, child.inodeid);
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
            {
                Logging::showErrorW("Maximum number of children in directory reached.");
                return -ENOMEM;
            }
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            return 0;
        }

        int FuseLink::unlink(const char *path)
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            APPFS_RETRIEVE_PATH_TO_INODE(child);

            // Ensure the INode is of the correct type.
            if (child.type == LowLevel::INodeType::INT_DIRECTORY)
                return -EISDIR;
            else if (child.type != LowLevel::INodeType::INT_FILEINFO && child.type != LowLevel::INodeType::INT_SYMLINK && child.type != LowLevel::INodeType::INT_DEVICE)
                return -EIO;

            // Get the INode's position.
            uint32_t pos = FuseLink::filesystem->getINodePositionByID(child.inodeid);
            if (pos == 0)
                return -EIO;

            // Get the real INode (if this is a hardlink).
            int result = 0;
            LowLevel::INode real = child.resolve(FuseLink::filesystem);
            if (real.inodeid != child.inodeid)
            {
                // The real inode is a hardlink, so we need to
                // reset that block.
                uint32_t rpos = FuseLink::filesystem->getINodePositionByID(real.inodeid);
                if (FuseLink::filesystem->resetBlock(rpos) != LowLevel::FSResult::E_SUCCESS)
                    result = -EIO;
                if (FuseLink::filesystem->setINodePositionByID(real.inodeid, 0) != LowLevel::FSResult::E_SUCCESS)
                    result = -EIO;
            }

            // Remove the inode from the directory.
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->removeChildFromDirectoryInode(parent.inodeid, real.inodeid);
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_INVALID_FILENAME)
                return -ENOENT;
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            // Now reduce the nlink value by 1.
            child.nlink -= 1;
            child.ctime = APPFS_TIME();
            if (child.nlink == 0)
            {
                // Erase all of the file segments first.
                FuseLink::filesystem->truncateFile(child.inodeid, 0);

                // Now delete the block at the specified position.
                if (FuseLink::filesystem->resetBlock(pos) != LowLevel::FSResult::E_SUCCESS)
                    return -EIO;
                if (FuseLink::filesystem->setINodePositionByID(child.inodeid, 0) != LowLevel::FSResult::E_SUCCESS)
                    return -EIO;
            }
            else
            {
                // Otherwise just save the new nlink value back.
                APPFS_SAVE_INODE(child);
            }
            return result;
        }

        int FuseLink::rmdir(const char *path)
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            APPFS_RETRIEVE_PATH_TO_INODE(child);

            // Ensure the INode is of the correct type.
            if (child.type != LowLevel::INodeType::INT_DIRECTORY)
                return -ENOTDIR;

            // Ensure the directory is empty.
            if (child.children_count != 0)
                return -ENOTEMPTY;

            // Get the position of the INode.
            uint32_t pos = FuseLink::filesystem->getINodePositionByID(child.inodeid);
            if (pos == 0)
                return -EIO;

            // Remove the directory from it's parent.
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->removeChildFromDirectoryInode(parent.inodeid, child.inodeid);
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_INVALID_FILENAME)
                return -ENOENT;
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            // Now delete the block at the specified position.
            if (FuseLink::filesystem->resetBlock(pos) != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            // Set the inode position to 0.
            if (FuseLink::filesystem->setINodePositionByID(child.inodeid, 0) != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            return 0;
        }

        int FuseLink::symlink(const char *target, const char *path)
        {
            APPFS_CHECK_PATH_NOT_EXISTS();

            int res = FuseLink::create(path, 0755, APPFS_MAKE_SYMLINK);
            if (res != 0)
                return res;
            res = FuseLink::write(path, target, strlen(target), 0, NULL);
            if (res < 0)
                return res;

            return 0;
        }

        int FuseLink::rename(const char *target, const char *path)
        {
            APPFS_CHECK_PATH_RENAMABILITY();
            APPFS_CHECK_TARGET_EXISTS();
            APPFS_RETRIEVE_TARGET_TO_INODE(child);
            APPFS_RETRIEVE_PARENT_TARGET_TO_INODE(old_parent);
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(new_parent);

            // Change directory owners if needed.
            if (old_parent.inodeid != new_parent.inodeid)
            {
                // Add the new parent -> child relationship on disk.
                LowLevel::FSResult::FSResult res = FuseLink::filesystem->addChildToDirectoryInode(new_parent.inodeid, child.inodeid);
                if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                    return -ENOTDIR;
                else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
                {
                    Logging::showErrorW("Maximum number of children in directory reached.");
                    return -EIO;
                }
                else if (res != LowLevel::FSResult::E_SUCCESS)
                    return -EIO;

                // Remove the old parent -> child relationship on disk.
                res = FuseLink::filesystem->removeChildFromDirectoryInode(old_parent.inodeid, child.inodeid);
                if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                    return -ENOTDIR;
                else if (res == LowLevel::FSResult::E_FAILURE_INVALID_FILENAME)
                    return -ENOENT;
                else if (res != LowLevel::FSResult::E_SUCCESS)
                    return -EIO;
            }

            // Now change the filename.
            child.ctime = APPFS_TIME();
            APPFS_BASENAME_TO_FILENAME(path, child.filename);
            APPFS_SAVE_INODE(child);

            return 0;
        }

        int FuseLink::link(const char *target, const char *path)
        {
            // Check to see if the path does not exist.
            APPFS_CHECK_PATH_NOT_EXISTS();

            // Check to see if the target exists.
            APPFS_CHECK_TARGET_EXISTS();

            // Retrieve inode of target.
            APPFS_RETRIEVE_TARGET_TO_INODE(real);

            // Ensure the target is a plain old file.
            if (real.type == LowLevel::INodeType::INT_DIRECTORY)
                return -EISDIR;
            else if (real.type != LowLevel::INodeType::INT_FILEINFO && real.type != LowLevel::INodeType::INT_DEVICE)
                return -ENOTSUP;

            // Retrieve inode of path's parent.
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);

            APPFS_ASSIGN_NEW_INODE(child, LowLevel::INodeType::INT_HARDLINK);
            child.realid = real.inodeid;
            APPFS_BASENAME_TO_FILENAME(path, child.filename);
            APPFS_SAVE_NEW_INODE(child);

            // Now add the parent -> child relationship on disk.
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->addChildToDirectoryInode(parent.inodeid, child.inodeid);
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
            {
                Logging::showErrorW("Maximum number of children in directory reached.");
                return -EIO;
            }
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            // Increase nlink and save.
            real.nlink += 1;
            real.ctime = APPFS_TIME();
            APPFS_SAVE_INODE(real);

            return 0;
        }

        int FuseLink::chmod(const char *path, mode_t mode)
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);
            buf.mask = Macros::extractMaskFromMode(mode);
            buf.ctime = APPFS_TIME();
            buf.atime = APPFS_TIME();
            APPFS_SAVE_INODE(buf);

            return 0;
        }

        int FuseLink::chown(const char *path, uid_t user, gid_t group)
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);
            buf.ctime = APPFS_TIME();
            buf.atime = APPFS_TIME();
            if (user != -1)
                buf.uid = user;
            if (group != -1)
                buf.gid = group;
            APPFS_SAVE_INODE(buf);

            return 0;
        }

        int FuseLink::truncate(const char *path, off_t size)
        {
            if (size > MSIZE_FILE)
                return -EFBIG;
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);
            buf.ctime = APPFS_TIME();
            buf.mtime = APPFS_TIME();
            buf.atime = APPFS_TIME();
            APPFS_SAVE_INODE(buf);
            FSFile file = FuseLink::filesystem->getFile(buf.inodeid);
            file.open();
            if (file.fail() && file.bad())
                return -EIO;
            file.truncate(size);
            if (file.fail() && file.bad())
                return -EIO;
            file.close();
            if (file.fail() && file.bad())
                return -EIO;

            return 0;
        }

        int FuseLink::open(const char *path, struct fuse_file_info *options)
        {
            APPFS_CHECK_PATH_EXISTS();

            return 0;	// Is there anything for us to do in the open() call?
        }

        int FuseLink::read(const char *path, char *out, size_t length, off_t offset, struct fuse_file_info *options)
        {
            if (offset > MSIZE_FILE || ((uint64_t) offset + (uint64_t) length) > MSIZE_FILE)
                return -EFBIG;
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);
            buf.atime = APPFS_TIME();
            APPFS_SAVE_INODE(buf);

#ifdef DEBUG
            Logging::showDebugW("Opening file with inode %u.", buf.inodeid);
#endif
            FSFile file = FuseLink::filesystem->getFile(buf.inodeid);
            file.open();
            if (file.fail() && file.bad())
                return -EIO;
#ifdef DEBUG
            Logging::showDebugW("Seeking to position %u.", offset);
#endif
            file.seekg(offset);
            if (file.fail() && file.bad())
                return -EIO;
#ifdef DEBUG
            Logging::showDebugW("Reading %u bytes.", length);
#endif
            uint32_t amount_read = file.read(out, length);
            if (file.fail() && file.bad())
                return -EIO;
#ifdef DEBUG
            Logging::showDebugW("Closing file.");
#endif
            file.close();
            if (file.fail() && file.bad())
                return -EIO;

#ifdef DEBUG
            Logging::showDebugW("%u bytes have been read.", amount_read);
#endif
            return amount_read;
        }

        int FuseLink::write(const char *path, const char *in, size_t length, off_t offset, struct fuse_file_info *options)
        {
            if (offset > MSIZE_FILE || ((uint64_t) offset + (uint64_t) length) > MSIZE_FILE)
                return -EFBIG;
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);
            buf.ctime = APPFS_TIME();
            buf.mtime = APPFS_TIME();
            buf.atime = APPFS_TIME();
            APPFS_SAVE_INODE(buf);

#ifdef DEBUG
            Logging::showDebugW("Opening file with inode %u.", buf.inodeid);
#endif
            FSFile file = FuseLink::filesystem->getFile(buf.inodeid);
            file.open();
            if (file.fail() && file.bad())
                return -EIO;
#ifdef DEBUG
            Logging::showDebugW("Seeking to position %u.", offset);
#endif
            file.seekp(offset);
            if (file.fail() && file.bad())
                return -EIO;
#ifdef DEBUG
            Logging::showDebugW("Writing %u bytes.", length);
#endif
            file.write(in, length);
            if (file.fail() && file.bad())
                return -EIO;
#ifdef DEBUG
            Logging::showDebugW("Closing file.");
#endif
            file.close();
            if (file.fail() && file.bad())
                return -EIO;

#ifdef DEBUG
            Logging::showDebugW("%u bytes have been written.", length);
#endif
            return length;
        }

        int FuseLink::statfs(const char *path, struct statvfs *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::flush(const char *path, struct fuse_file_info *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::release(const char *path, struct fuse_file_info *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::fsync(const char *path, int, struct fuse_file_info *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::opendir(const char *path, struct fuse_file_info *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::readdir(const char *path, void *dbuf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);

            // Check to make sure the inode is a directory.
            if (buf.type != LowLevel::INodeType::INT_DIRECTORY)
            {
                return -ENOTDIR;
            }

            // Retrieve the INode's children.
            std::vector < LowLevel::INode > children = FuseLink::filesystem->getChildrenOfDirectory(buf.inodeid);

            // Use the filler() function to report the entries back to FUSE.
            filler(dbuf, ".", NULL, 0);
            filler(dbuf, "..", NULL, 0);
            for (int i = 0; i < children.size(); i += 1)
            {
                filler(dbuf, children[i].filename, NULL, 0);
            }

            return 0;
        }

        int FuseLink::releasedir(const char *path, struct fuse_file_info *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::fsyncdir(const char *path, int, struct fuse_file_info *)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        void *FuseLink::init(struct fuse_conn_info *conn)
        {
            if (FuseLink::continuefunc != NULL)
            {
                FuseLink::continuefunc();
            }
            return NULL;
        }

        void FuseLink::destroy(void *)
        {
            return;
        }

        int FuseLink::access(const char *path, int)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::create(const char *path, mode_t mode, struct fuse_file_info *options)
        {
            APPFS_CHECK_PATH_NOT_EXISTS();
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            LowLevel::INodeType::INodeType type = LowLevel::INodeType::INT_FILEINFO;
            if (options == APPFS_MAKE_SYMLINK)
            {
                type = LowLevel::INodeType::INT_SYMLINK;
                options = NULL;
            }
            APPFS_ASSIGN_NEW_INODE(child, type);
            child.mask = Macros::extractMaskFromMode(mode);
            child.ctime = APPFS_TIME();
            child.mtime = APPFS_TIME();
            child.atime = APPFS_TIME();
            child.uid = fuse_get_context()->uid;
            child.gid = fuse_get_context()->gid;
            APPFS_BASENAME_TO_FILENAME(path, child.filename);
            APPFS_SAVE_NEW_INODE(child);

            // Now add the parent -> child relationship on disk.
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->addChildToDirectoryInode(parent.inodeid, child.inodeid);
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
                return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
            {
                Logging::showErrorW("Maximum number of children in directory reached.");
                return -EIO;
            }
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            return 0;
        }

        int FuseLink::ftruncate(const char *path, off_t len, struct fuse_file_info *options)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::fgetattr(const char *path, struct stat *st, struct fuse_file_info *options)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::utimens(const char *path, const struct timespec tv[2])
        {
            APPFS_CHECK_PATH_EXISTS();
            APPFS_RETRIEVE_PATH_TO_INODE(buf);
            buf.atime = tv[0].tv_sec;
            buf.atime = tv[1].tv_sec;
            APPFS_SAVE_INODE(buf);

            return 0;
        }

        int FuseLink::ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int FuseLink::poll(const char *path, struct fuse_file_info *options, struct fuse_pollhandle *ph, unsigned *reventsp)
        {
            APPFS_CHECK_PATH_EXISTS();

            return -ENOTSUP;
        }

        int Macros::checkPathExists(const char *path)
        {
            std::vector < std::string > ret = FuseLink::filesystem->splitPathBySeperators(path);
            LowLevel::INode buf = FuseLink::filesystem->getINodeByID(0);
            for (unsigned int i = 0; i < ret.size(); i += 1)
            {
                buf = FuseLink::filesystem->getChildOfDirectory(buf.inodeid, ret[i].c_str());
                if (buf.type == LowLevel::INodeType::INT_INVALID)
                {
                    return -ENOENT;
                }
            }
            if (buf.type == LowLevel::INodeType::INT_INVALID)
            {
                return -ENOENT;
            }
            return 0;
        }

        int Macros::checkPathNotExists(const char *path)
        {
            // Assumes the context is file creation, so returns
            // -ENOENT if any of the components of the path
            // except the last one do not exist.  Returns -EEXIST
            // if the entire path exists.  Returns 0 if all except
            // the last component of the path exists.
            std::vector < std::string > ret = FuseLink::filesystem->splitPathBySeperators(path);
            LowLevel::INode buf = FuseLink::filesystem->getINodeByID(0);
            for (unsigned int i = 0; i < ret.size() - 1; i += 1)
            {
                buf = FuseLink::filesystem->getChildOfDirectory(buf.inodeid, ret[i].c_str());
                if (buf.type == LowLevel::INodeType::INT_INVALID)
                {
                    return -ENOENT;
                }
            }
            if (ret.size() != 0)
                buf = FuseLink::filesystem->getChildOfDirectory(buf.inodeid, ret[ret.size() - 1].c_str());
            if (buf.type != LowLevel::INodeType::INT_INVALID)
            {
                return -EEXIST;
            }
            return 0;
        }

        int Macros::checkPathIsValid(const char *path)
        {
            std::vector < std::string > pp = FuseLink::filesystem->splitPathBySeperators(path);
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->verifyPath(path, pp);
            if (res == LowLevel::FSResult::E_FAILURE_INVALID_FILENAME)
                return -ENAMETOOLONG;
            else if (res == LowLevel::FSResult::E_FAILURE_INVALID_PATH)
                return -ENAMETOOLONG;
            else if (res != LowLevel::FSResult::E_SUCCESS)
                return -EIO;

            return 0;
        }

        int Macros::checkPathRenamability(const char *path)
        {
            // If the path doesn't exist, we don't need to check the special
            // sticky conditions.
            int ret = Macros::checkPathNotExists(path);
            if (ret != -EEXIST)
                return ret;

            // So the path does exist, and we need to check the modes on the
            // owning directory and the file.
            APPFS_RETRIEVE_PATH_TO_INODE(child);
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            uid_t uid = fuse_get_context()->uid;
            if ((parent.mask & S_ISVTX) && !(child.uid == uid || parent.uid == uid))
                return -EACCES;

            // Otherwise, the user is permitted to do this, but we have to remove
            // the existing entry first.
            if (child.type == LowLevel::INodeType::INT_DIRECTORY)
                return FuseLink::rmdir(path);
            else
                return FuseLink::unlink(path);
        }

        int Macros::checkPermission(const char *path, char op, int uid, int gid)
        {
            return -EACCES;
        }

        int Macros::retrievePathToINode(const char *path, LowLevel::INode * out)
        {
            std::vector < std::string > ret = FuseLink::filesystem->splitPathBySeperators(path);
            LowLevel::INode buf = FuseLink::filesystem->getINodeByID(0);
            for (unsigned int i = 0; i < ret.size(); i += 1)
            {
                buf = FuseLink::filesystem->getChildOfDirectory(buf.inodeid, ret[i].c_str());
                if (buf.type == LowLevel::INodeType::INT_INVALID)
                {
                    APPFS_COPY_INODE_TO_POINTER(buf, out);
                    return -ENOENT;
                }
            }
            if (buf.type == LowLevel::INodeType::INT_INVALID)
            {
                APPFS_COPY_INODE_TO_POINTER(buf, out);
                return -ENOENT;
            }
            if (buf.type == LowLevel::INodeType::INT_HARDLINK)
                buf = buf.resolve(FuseLink::filesystem);
            APPFS_COPY_INODE_TO_POINTER(buf, out);
            return 0;
        }

        int Macros::retrieveParentPathToINode(const char *path, LowLevel::INode * out)
        {
            std::vector < std::string > ret = FuseLink::filesystem->splitPathBySeperators(path);
            LowLevel::INode buf = FuseLink::filesystem->getINodeByID(0);
            for (unsigned int i = 0; i < ret.size() - 1; i += 1)
            {
                buf = FuseLink::filesystem->getChildOfDirectory(buf.inodeid, ret[i].c_str());
                if (buf.type == LowLevel::INodeType::INT_INVALID)
                {
                    APPFS_COPY_INODE_TO_POINTER(buf, out);
                    return -ENOENT;
                }
            }
            if (buf.type == LowLevel::INodeType::INT_INVALID)
            {
                APPFS_COPY_INODE_TO_POINTER(buf, out);
                return -ENOENT;
            }
            APPFS_COPY_INODE_TO_POINTER(buf, out);
            return 0;
        }

        int Macros::saveINode(LowLevel::INode * buf)
        {
            if (buf->type == LowLevel::INodeType::INT_INVALID || buf->type == LowLevel::INodeType::INT_UNSET)
            {
                Logging::showErrorW("Attempted to update UNSET or INVALID inode.");
                return -EIO;
            }
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->updateINode(*buf);
            if (res != LowLevel::FSResult::E_SUCCESS)
            {
                Logging::showErrorW("Unable to update inode on disk.");
                return -EIO;
            }
            return 0;
        }

        int Macros::saveNewINode(uint32_t pos, LowLevel::INode * buf)
        {
            if (buf->type == LowLevel::INodeType::INT_INVALID || buf->type == LowLevel::INodeType::INT_UNSET)
            {
                Logging::showErrorW("Attempted to write UNSET or INVALID inode.");
                return -EIO;
            }
            LowLevel::FSResult::FSResult res = FuseLink::filesystem->writeINode(pos, *buf);
            if (res != LowLevel::FSResult::E_SUCCESS)
            {
                Logging::showErrorW("Unable to write inode on disk.");
                return -EIO;
            }
            return 0;
        }

        int Macros::assignNewINode(LowLevel::INode * buf, LowLevel::INodeType::INodeType type, uint32_t & pos)
        {
            if (type == LowLevel::INodeType::INT_INVALID || type == LowLevel::INodeType::INT_UNSET)
            {
                Logging::showErrorW("Can't write INVALID or UNSET inodes to disk.");
                return -EIO;
            }
            LowLevel::INodeType::INodeType simple_type = LowLevel::INodeType::INT_FILEINFO;
            if (type == LowLevel::INodeType::INT_DIRECTORY)
                simple_type = LowLevel::INodeType::INT_DIRECTORY;

            // Get a free block.
            pos = FuseLink::filesystem->getFirstFreeBlock(simple_type);
            if (pos == 0)
                return -ENOSPC;
            APPFS_VERIFY_INODE_POSITION_CUSTOM(pos, -EIO);

            // Get a free inode number.
            uint16_t id = FuseLink::filesystem->getFirstFreeINodeNumber();
            if (id == 0)
                return -ENOSPC;
            buf->inodeid = id;

            // Ensure that we don't have an invalid position and that the
            // INode ID is not already assigned.
            if (pos < OFFSET_DATA)
                return LowLevel::FSResult::E_FAILURE_INVALID_POSITION;
            if (type != LowLevel::INodeType::INT_SEGINFO && type != LowLevel::INodeType::INT_FREELIST && FuseLink::filesystem->getINodePositionByID(id) != 0)
                return LowLevel::FSResult::E_FAILURE_INODE_ALREADY_ASSIGNED;

            // Associate the INode ID with this INode's position.
            FuseLink::filesystem->reserveINodeID(id);

            // XXX: We used to write the INode to disk at this point however that left
            //      partial entries on disk in some cases.  Are we creating a race condition
            //      where two different INodes can have the same ID if we don't write at
            //      this point?
            return 0;

            /*
             * LowLevel::FSResult::FSResult res = FuseLink::filesystem->writeINode(pos, *buf);
             * if (res == LowLevel::FSResult::E_FAILURE_INVALID_POSITION)
             * {
             * Logging::showErrorW("Invalid position for assigning new inode.");
             * return -EIO;
             * }
             * else if (res == LowLevel::FSResult::E_FAILURE_INODE_ALREADY_ASSIGNED)
             * {
             * Logging::showErrorW("INode number (%i) has already been assigned.", buf->inodeid);
             * return -EIO;
             * }
             * else if (res == LowLevel::FSResult::E_SUCCESS)
             * return 0;
             */
        }

        int Macros::extractMaskFromMode(mode_t mode)
        {
            if (mode & S_IFDIR)
                return mode & ~S_IFREG;
            else if (mode & S_IFLNK)
                return mode & ~S_IFLNK;
            else if (mode & S_IFREG)
                return mode & ~S_IFREG;
            else
                return mode;	// Keep block / char / FIFO information.
        }

        const char *Macros::extractBasenameFromPath(const char *path)
        {
            std::vector < std::string > pret = FuseLink::filesystem->splitPathBySeperators(path);
            if (pret.size() == 0)
                return "";
            std::string pstr = pret[pret.size() - 1];
            return strdup(pstr.c_str());
        }
    }
}