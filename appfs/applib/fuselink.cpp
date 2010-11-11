/*

Code file for FuseLink.

This class recieves callbacks from FUSE and handles each operation
by passing them to the actual FS class in a C++ friendly way.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                25th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"
#include "fuselink.h"
#include "logging.h"
#include "fsmacro.h"
#include <string>
#include <time.h>

namespace AppLib
{
	namespace FUSE
	{
		LowLevel::FS * FuseLink::filesystem = NULL;
		void (*FuseLink::continuefunc)(void) = NULL;

		Mounter::Mounter(const char *disk_image,
						const char *mount_point,
						bool foreground,
						void (*continuefunc)(void))
		{
#ifdef WIN32
			this->mountResult = -ENOTSUP;
#else
			this->mountResult = -EALREADY;

			// Define the fuse_operations structure.
			static fuse_operations appfs_ops;
			appfs_ops.getattr		= &FuseLink::getattr;
//			appfs_ops.readlink		= &FuseLink::readlink;
//			appfs_ops.mknod			= &FuseLink::mknod;
			appfs_ops.mkdir			= &FuseLink::mkdir;
			appfs_ops.unlink		= &FuseLink::unlink;
			appfs_ops.rmdir			= &FuseLink::rmdir;
//			appfs_ops.symlink		= &FuseLink::symlink;
//			appfs_ops.rename		= &FuseLink::rename;
//			appfs_ops.link			= &FuseLink::link;
			appfs_ops.chmod			= &FuseLink::chmod;
			appfs_ops.chown			= &FuseLink::chown;
			appfs_ops.truncate		= &FuseLink::truncate;
			appfs_ops.open			= &FuseLink::open;
			appfs_ops.read			= &FuseLink::read;
			appfs_ops.write			= &FuseLink::write;
//			appfs_ops.statfs		= &FuseLink::statfs;
//			appfs_ops.flush			= &FuseLink::flush;
//			appfs_ops.release		= &FuseLink::release;
//			appfs_ops.fsync			= &FuseLink::fsync;
			appfs_ops.setxattr		= NULL;
			appfs_ops.getxattr		= NULL;
			appfs_ops.listxattr		= NULL;
			appfs_ops.removexattr		= NULL;
//			appfs_ops.opendir		= &FuseLink::opendir;
			appfs_ops.readdir		= &FuseLink::readdir;
//			appfs_ops.releasedir		= &FuseLink::releasedir;
//			appfs_ops.fsyncdir		= &FuseLink::fsyncdir;
			appfs_ops.init			= &FuseLink::init;
			appfs_ops.destroy		= &FuseLink::destroy;
//			appfs_ops.access		= &FuseLink::access;
			appfs_ops.create		= &FuseLink::create;
//			appfs_ops.ftruncate		= &FuseLink::ftruncate;
//			appfs_ops.fgetattr		= &FuseLink::fgetattr;
			appfs_ops.lock			= NULL;
			appfs_ops.utimens		= &FuseLink::utimens;
			appfs_ops.bmap			= NULL;

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
				if (fuse_opt_add_arg(&fargs, "-f") == -1 ||
				    fuse_opt_add_arg(&fargs, "-d") == -1)
				{
					Logging::showErrorW("Unable to set FUSE options.");
	                                fuse_opt_free_args(&fargs);
	                                this->mountResult = -5;
	                                return;
				}
			}

			if (fuse_opt_add_arg(&fargs, "-o") == -1 ||
				fuse_opt_add_arg(&fargs, "default_permissions") == -1 ||
				fuse_opt_add_arg(&fargs, mount_point) == -1)
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

		int FuseLink::getattr(const char* path, struct stat *stbuf)
		{
			int result = 0;
			
			// Create a new stat object in the stbuf position.
			memset(stbuf, 0, sizeof(struct stat));

			std::vector<std::string> ret = FuseLink::filesystem->splitPathBySeperators(path);
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

			// Set the values of the stat structure.
			stbuf->st_dev = buf.dev;
			stbuf->st_mode = buf.mask;
			stbuf->st_nlink = buf.nlink;
			stbuf->st_uid = buf.uid;
			stbuf->st_gid = buf.gid;
			stbuf->st_rdev = buf.rdev;
			if (buf.type == LowLevel::INodeType::INT_FILEINFO)
			{
				stbuf->st_size = buf.dat_len;
#ifndef WIN32
				stbuf->st_blksize = BSIZE_FILE;
				stbuf->st_blocks = buf.blocks;
#endif
				stbuf->st_mode = S_IFREG | stbuf->st_mode;
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

		int FuseLink::readlink(const char * path, char * out, size_t size)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::mknod(const char * path, mode_t mask, dev_t devid)
		{
			APPFS_CHECK_PATH_NOT_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::mkdir(const char * path, mode_t mode)
		{
			APPFS_CHECK_PATH_NOT_EXISTS();
			APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
			APPFS_ASSIGN_NEW_INODE(child, LowLevel::INodeType::INT_DIRECTORY);
			child.mask = Macros::extractMaskFromMode(mode);
			child.ctime = APPFS_TIME();
			child.mtime = APPFS_TIME();
			child.atime = APPFS_TIME();
			APPFS_COPY_CONST_CHAR_TO_FILENAME(Macros::extractBasenameFromPath(path), child.filename);
			LowLevel::FSResult::FSResult res =
				FuseLink::filesystem->addChildToDirectoryInode(parent.inodeid, child.inodeid);
			// TODO: If we return an error, we must also delete the child inode
			//       that we created with APPFS_ASSIGN_NEW_INODE().  We need to
			//       create a new macro to handle this situation.
			if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
				return -ENOTDIR;
			else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
			{
				Logging::showErrorW("Maximum number of children in directory reached.");
				return -EIO;
			}
			
			APPFS_SAVE_INODE(child);

			return 0;
		}

		int FuseLink::unlink(const char * path)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
			APPFS_RETRIEVE_PATH_TO_INODE(child);
			
			// Ensure the INode is of the correct type.
			if (child.type == LowLevel::INodeType::INT_DIRECTORY)
				return -EISDIR;
			else if (child.type != LowLevel::INodeType::INT_FILEINFO &&
			         child.type != LowLevel::INodeType::INT_SYMLINK)
				return -EIO;

			// Get the INode's position.
			uint32_t pos = FuseLink::filesystem->getINodePositionByID(child.inodeid);
			if (pos == 0)
				return -EIO;

			// Erase all of the file segments first.
			uint32_t npos = FuseLink::filesystem->getFileNextBlock(child.inodeid, pos);
			uint32_t opos = npos;
			while (npos != 0)
			{
				npos = FuseLink::filesystem->getFileNextBlock(child.inodeid, npos);
				FuseLink::filesystem->resetBlock(opos);
				opos = npos;
			}

			// Remove the inode from the directory.
			LowLevel::FSResult::FSResult res =
				FuseLink::filesystem->removeChildFromDirectoryInode(parent.inodeid, child.inodeid);
			if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
				return -ENOTDIR;
			else if (res == LowLevel::FSResult::E_FAILURE_INVALID_FILENAME)
				return -ENOENT;

			// Now delete the block at the specified position.
			FuseLink::filesystem->resetBlock(pos);

			return 0;
		}

		int FuseLink::rmdir(const char * path)
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
			LowLevel::FSResult::FSResult res =
				FuseLink::filesystem->removeChildFromDirectoryInode(parent.inodeid, child.inodeid);
			if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
				return -ENOTDIR;
			else if (res == LowLevel::FSResult::E_FAILURE_INVALID_FILENAME)
				return -ENOENT;

			// Now delete the block at the specified position.
			FuseLink::filesystem->resetBlock(pos);

			return 0;
		}

		int FuseLink::symlink(const char * path, const char * source)
		{
			APPFS_CHECK_PATH_NOT_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::rename(const char * path, const char * dest)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::link(const char * path, const char * source)
		{
			APPFS_CHECK_PATH_NOT_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::chmod(const char * path, mode_t mode)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PATH_TO_INODE(buf);
			buf.mask = Macros::extractMaskFromMode(mode);
                        buf.mtime = APPFS_TIME();
                        buf.atime = APPFS_TIME();
			APPFS_SAVE_INODE(buf);

			return 0;
		}

		int FuseLink::chown(const char * path, uid_t user, gid_t group)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PATH_TO_INODE(buf);
			buf.mtime = APPFS_TIME();
                        buf.atime = APPFS_TIME();
			if (user != -1)
				buf.uid = user;
			if (group != -1)
				buf.gid = group;
			APPFS_SAVE_INODE(buf);

			return 0;
		}

		int FuseLink::truncate(const char * path, off_t size)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PATH_TO_INODE(buf);
			buf.mtime = APPFS_TIME();
            buf.atime = APPFS_TIME();
            APPFS_SAVE_INODE(buf);
            LowLevel::FSFile file = FuseLink::filesystem->getFile(buf.inodeid);
            file.open();
            if (file.fail() && file.bad()) return -EIO;
            file.truncate(size);
            if (file.fail() && file.bad()) return -EIO;
            file.close();
            if (file.fail() && file.bad()) return -EIO;

            return 0;
		}

		int FuseLink::open(const char * path, struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();

			return 0; // Is there anything for us to do in the open() call?
		}

		int FuseLink::read(const char * path, char * out, size_t length, off_t offset,
						struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PATH_TO_INODE(buf);
                        buf.atime = APPFS_TIME();
			APPFS_SAVE_INODE(buf);

			std::stringstream ss1; ss1 << "Opening file with inode " << buf.inodeid << ".";
                        Logging::showInfoW(ss1.str());
			LowLevel::FSFile file = FuseLink::filesystem->getFile(buf.inodeid);
			file.open();
			if (file.fail() && file.bad()) return -EIO;
			std::stringstream ss2; ss2 << "Seeking to position " << offset << ".";
                        Logging::showInfoW(ss2.str());
			file.seekg(offset);
			if (file.fail() && file.bad()) return -EIO;
			std::stringstream ss3; ss3 << "Reading " << length << " bytes.";
                        Logging::showInfoW(ss3.str());
			uint32_t amount_read = file.read(out, length);
			if (file.fail() && file.bad()) return -EIO;
			std::stringstream ss4; ss4 << "Closing file.";
                        Logging::showInfoW(ss4.str());
			file.close();
			if (file.fail() && file.bad()) return -EIO;

			std::stringstream ss5; ss5 << length << " bytes have been read.";
                        Logging::showInfoW(ss5.str());
			return amount_read;
		}

		int FuseLink::write(const char * path, const char * in, size_t length, off_t offset,
						struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PATH_TO_INODE(buf);
			buf.mtime = APPFS_TIME();
			buf.atime = APPFS_TIME();
			APPFS_SAVE_INODE(buf);

			std::stringstream ss1; ss1 << "Opening file with inode " << buf.inodeid << ".";
			Logging::showInfoW(ss1.str());
			LowLevel::FSFile file = FuseLink::filesystem->getFile(buf.inodeid);
			file.open();
			if (file.fail() && file.bad()) return -EIO;
			std::stringstream ss2; ss2 << "Seeking to position " << offset << ".";
			Logging::showInfoW(ss2.str());
			file.seekp(offset);
			if (file.fail() && file.bad()) return -EIO;
			std::stringstream ss3; ss3 << "Writing " << length << " bytes.";
			Logging::showInfoW(ss3.str());
			file.write(in, length);
			if (file.fail() && file.bad()) return -EIO;
			std::stringstream ss4; ss4 << "Closing file.";
			Logging::showInfoW(ss4.str());
			file.close();
			if (file.fail() && file.bad()) return -EIO;

			std::stringstream ss5; ss5 << length << " bytes have been written.";
			Logging::showInfoW(ss5.str());
			return length;
		}

		int FuseLink::statfs(const char * path, struct statvfs *)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::flush(const char * path, struct fuse_file_info *)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::release(const char * path, struct fuse_file_info *)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::fsync(const char * path, int, struct fuse_file_info *)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::opendir(const char * path, struct fuse_file_info *)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::readdir(const char * path, void * dbuf, fuse_fill_dir_t filler, off_t offset,
						struct fuse_file_info * fi)
		{
			APPFS_CHECK_PATH_EXISTS();
			APPFS_RETRIEVE_PATH_TO_INODE(buf);

			// Check to make sure the inode is a directory.
			if (buf.type != LowLevel::INodeType::INT_DIRECTORY)
			{
				return -ENOTDIR;
			}

			// Retrieve the INode's children.
			std::vector<LowLevel::INode> children = FuseLink::filesystem->getChildrenOfDirectory(buf.inodeid);

			// Use the filler() function to report the entries back to FUSE.
			filler(dbuf, ".", NULL, 0);
			filler(dbuf, "..", NULL, 0);
			for (int i = 0; i < children.size(); i += 1)
			{
				filler(dbuf, children[i].filename, NULL, 0);
			}

			return 0;
		}

		int FuseLink::releasedir(const char * path, struct fuse_file_info *)
		{
			APPFS_CHECK_PATH_EXISTS();
			
			return -ENOTSUP;
		}

		int FuseLink::fsyncdir(const char * path, int, struct fuse_file_info *)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		void * FuseLink::init(struct fuse_conn_info *conn)
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

		int FuseLink::access(const char * path, int)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::create(const char * path, mode_t mode, struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_NOT_EXISTS();
            APPFS_RETRIEVE_PARENT_PATH_TO_INODE(parent);
            APPFS_ASSIGN_NEW_INODE(child, LowLevel::INodeType::INT_FILEINFO);
            child.mask = Macros::extractMaskFromMode(mode);
            child.ctime = APPFS_TIME();
            child.mtime = APPFS_TIME();
            child.atime = APPFS_TIME();
			const char* ret_cpy = Macros::extractBasenameFromPath(path);
			/* ret may be the result of a function. */ \
			for (int i = 0; i < 256; i += 1)
			        child.filename[i] = 0;
			for (int i = 0; i < strlen(ret_cpy); i += 1)
			{
			        if (ret_cpy[i] == 0)
			                break;
			        child.filename[i] = ret_cpy[i];
			}
            LowLevel::FSResult::FSResult res =
                    FuseLink::filesystem->addChildToDirectoryInode(parent.inodeid, child.inodeid);
            // TODO: If we return an error, we must also delete the child inode
            //       that we created with APPFS_ASSIGN_NEW_INODE().  We need to
            //       create a new macro to handle this situation.
            if (res == LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY)
				return -ENOTDIR;
            else if (res == LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED)
            {
                Logging::showErrorW("Maximum number of children in directory reached.");
                return -EIO;
            }

            APPFS_SAVE_INODE(child);

            return 0;
		}

		int FuseLink::ftruncate(const char * path, off_t len, struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::fgetattr(const char * path, struct stat * st, struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::utimens(const char * path, const struct timespec tv[2])
		{
			APPFS_CHECK_PATH_EXISTS();
			/*APPFS_RETRIEVE_PATH_TO_INODE(buf);
                        buf.mtime = APPFS_TIME();
                        buf.atime = APPFS_TIME();
                        APPFS_SAVE_INODE(buf);*/

			return 0;
		}

		int FuseLink::ioctl(const char * path, int cmd, void *arg,
						struct fuse_file_info *, unsigned int flags, void *data)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::poll(const char * path, struct fuse_file_info * options,
						struct fuse_pollhandle *ph, unsigned *reventsp)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int Macros::checkPathExists(const char *path)
		{
			std::vector<std::string> ret = FuseLink::filesystem->splitPathBySeperators(path);
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

		int Macros::checkPathNotExists(const char* path)
		{
			// Assumes the context is file creation, so returns
			// -ENOENT if any of the components of the path
			// except the last one do not exist.  Returns -EEXIST
			// if the entire path exists.  Returns 0 if all except
			// the last component of the path exists.
			std::vector<std::string> ret = FuseLink::filesystem->splitPathBySeperators(path);
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
			return 0;
		}

		int Macros::checkPermission(const char *path, char op, int uid, int gid)
		{
			return -EACCES;
		}

		int Macros::retrievePathToINode(const char *path, LowLevel::INode * out)
		{
			std::vector<std::string> ret = FuseLink::filesystem->splitPathBySeperators(path);
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
			APPFS_COPY_INODE_TO_POINTER(buf, out);
			return 0;
		}

		int Macros::retrieveParentPathToINode(const char *path, LowLevel::INode * out)
        {
            std::vector<std::string> ret = FuseLink::filesystem->splitPathBySeperators(path);
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
			if (buf->type == LowLevel::INodeType::INT_INVALID ||
				buf->type == LowLevel::INodeType::INT_UNSET)
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

		int Macros::assignNewINode(LowLevel::INode* buf, LowLevel::INodeType::INodeType type)
		{
			if (type == LowLevel::INodeType::INT_INVALID ||
				type == LowLevel::INodeType::INT_UNSET)
			{
				Logging::showErrorW("Can't write INVALID or UNSET inodes to disk.");
				return -EIO;
			}
			LowLevel::INodeType::INodeType simple_type = LowLevel::INodeType::INT_FILEINFO;
			if (type == LowLevel::INodeType::INT_DIRECTORY)
				simple_type = LowLevel::INodeType::INT_DIRECTORY;

			// Get a free block.
			uint32_t pos = FuseLink::filesystem->getFirstFreeBlock(simple_type);
			if (pos == 0)
				return -ENOSPC;
			
			// Get a free inode number.
			uint16_t id = FuseLink::filesystem->getFirstFreeInodeNumber();
			if (id == 0)
				return -ENOSPC;
			buf->inodeid = id;

			LowLevel::FSResult::FSResult res = FuseLink::filesystem->writeINode(pos, *buf);
			if (res == LowLevel::FSResult::E_FAILURE_INVALID_POSITION)
			{
				Logging::showErrorW("Invalid position for assigning new inode.");
				return -EIO;
			}
			else if (res == LowLevel::FSResult::E_FAILURE_INODE_ALREADY_ASSIGNED)
			{
				Logging::showErrorW("INode number (%i) has already been assigned.", buf->inodeid);
				return -EIO;
			}
			else if (res == LowLevel::FSResult::E_SUCCESS)
				return 0;
		}
		
		int Macros::extractMaskFromMode(mode_t mode)
		{
			return mode - ((((((mode >> 3) >> 3) >> 3) << 3) << 3) << 3);
		}

		const char* Macros::extractBasenameFromPath(const char* path)
		{
			std::vector<std::string> pret = FuseLink::filesystem->splitPathBySeperators(path);
			if (pret.size() == 0) return "";
			std::string pstr = pret[pret.size() - 1];
			return pstr.c_str();
		}
	}
}
