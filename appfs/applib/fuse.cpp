/*

Code file for FuseLink.

This class recieves callbacks from FUSE and handles each operation
by passing them to the actual FS class in a C++ friendly way.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"
#include "fuse.h"
#include "fsmacro.h"
#include <string>

namespace AppLib
{
	namespace FUSE
	{
		LowLevel::FS FuseLink::filesystem = NULL;

		Mounter::Mounter(const char *disk_image,
						const char *mount_path,
						bool foreground,
						void (*continue_func)(void))
		{
			this->mountResult = -EALREADY;
			// Define the fuse_operations structure.
			static fuse_operations appfs_ops;
			appfs_ops.getattr		= &FuseLink::getattr;
			appfs_ops.readlink		= &FuseLink::readlink;
			appfs_ops.mknod			= &FuseLink::mknod;
			appfs_ops.mkdir			= &FuseLink::mkdir;
			appfs_ops.unlink		= &FuseLink::unlink;
			appfs_ops.rmdir			= &FuseLink::rmdir;
			appfs_ops.symlink		= &FuseLink::symlink;
			appfs_ops.rename		= &FuseLink::rename;
			appfs_ops.link			= &FuseLink::link;
			appfs_ops.chmod			= &FuseLink::chmod;
			appfs_ops.chown			= &FuseLink::chown;
			appfs_ops.truncate		= &FuseLink::truncate;
			appfs_ops.open			= &FuseLink::open;
			appfs_ops.read			= &FuseLink::read;
			appfs_ops.write			= &FuseLink::write;
			appfs_ops.statfs		= &FuseLink::statfs;
			appfs_ops.flush			= &FuseLink::flush;
			appfs_ops.release		= &FuseLink::release;
			appfs_ops.fsync			= &FuseLink::fsync;
			appfs_ops.setxattr		= NULL;
			appfs_ops.getxattr		= NULL;
			appfs_ops.listxattr		= NULL;
			appfs_ops.removexattr	= NULL;
			appfs_ops.opendir		= &FuseLink::opendir;
			appfs_ops.readdir		= &FuseLink::readdir;
			appfs_ops.releasedir	= &FuseLink::releasedir;
			appfs_ops.fsyncdir		= &FuseLink::fsyncdir;
			appfs_ops.init			= &FuseLink::init;
			appfs_ops.destroy		= &FuseLink::destroy;
			appfs_ops.access		= &FuseLink::access;
			appfs_ops.create		= &FuseLink::create;
			appfs_ops.ftruncate		= &FuseLink::ftruncate;
			appfs_ops.fgetattr		= &FuseLink::fgetattr;
			appfs_ops.lock			= NULL;
			appfs_ops.utimens		= &FuseLink::utimens;
			appfs_ops.bmap			= NULL;

			// Mounts the specified disk image at the
			// specified mount path using FUSE.
			struct fuse_args fargs = FUSE_ARGS_INIT(0, NULL);

			this->mountResult = fuse_main(fargs.argc, fargs.argv, &appfs_ops, NULL);
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

			std::vector<std::string> ret = FuseLink::filesystem.splitPathBySeperators(path);
			LowLevel::INode buf = FuseLink::filesystem.getINodeByID(0);
			for (unsigned int i = 0; i < ret.size(); i += 1)
			{
				buf = FuseLink::filesystem.getChildOfDirectory(buf.inodeid, ret[i].c_str());
				if (buf.type == LowLevel::INodeType::INT_INVALID)
				{
					return -ENOENT;
				}
			}

			// Set the values of the stat structure.
			stbuf->st_dev = buf.dev;
			stbuf->st_mode = buf.mask;
			stbuf->st_nlink = buf.nlink;
			stbuf->st_uid = buf.uid;
			stbuf->st_gid = buf.gid;
			stbuf->st_rdev = buf.rdev;
			if (buf.type == LowLevel::INodeType::INT_FILE)
			{
				stbuf->st_size = buf.dat_len;
#ifndef WIN32
				stbuf->st_blksize = BSIZE_FILE;
				stbuf->st_blocks = buf.blocks;
#endif
			}
			else if (buf.type == LowLevel::INodeType::INT_DIRECTORY)
			{
				stbuf->st_size = BSIZE_DIRECTORY;
#ifndef WIN32
				stbuf->st_blksize = BSIZE_FILE;
				stbuf->st_blocks = BSIZE_DIRECTORY / BSIZE_FILE;
#endif
			}
			stbuf->st_atime = buf.atime;
			stbuf->st_mtime = buf.mtime;
			stbuf->st_ctime = buf.ctime;

			// TODO: Implement this function.

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

		int FuseLink::mkdir(const char * path, mode_t mask)
		{
			APPFS_CHECK_PATH_NOT_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::unlink(const char * path)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::rmdir(const char * path)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
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

			return -ENOTSUP;
		}

		int FuseLink::chown(const char * path, uid_t user, gid_t group)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::truncate(const char * path, off_t size)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::open(const char * path, struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::read(const char * path, char * out, size_t length, off_t offset,
						struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
		}

		int FuseLink::write(const char * path, const char * in, size_t length, off_t offset,
						struct fuse_file_info * options)
		{
			APPFS_CHECK_PATH_EXISTS();

			return -ENOTSUP;
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
			return -ENOTSUP;
		}

		int FuseLink::fsync(const char * path, int, struct fuse_file_info *)
		{
			return -ENOTSUP;
		}

		int FuseLink::opendir(const char * path, struct fuse_file_info *)
		{
			return -ENOTSUP;
		}

		int FuseLink::readdir(const char * path, void *, fuse_fill_dir_t, off_t,
						struct fuse_file_info *)
		{
			return -ENOTSUP;
		}

		int FuseLink::releasedir(const char * path, struct fuse_file_info *)
		{
			return -ENOTSUP;
		}

		int FuseLink::fsyncdir(const char * path, int, struct fuse_file_info *)
		{
			return -ENOTSUP;
		}

		void * FuseLink::init(struct fuse_conn_info *conn)
		{
			return NULL;
		}

		void FuseLink::destroy(void *)
		{
			return;
		}

		int FuseLink::access(const char * path, int)
		{
			return -ENOTSUP;
		}

		int FuseLink::create(const char * path, mode_t mode, struct fuse_file_info * options)
		{
			return -ENOTSUP;
		}

		int FuseLink::ftruncate(const char * path, off_t len, struct fuse_file_info * options)
		{
			return -ENOTSUP;
		}

		int FuseLink::fgetattr(const char * path, struct stat * st, struct fuse_file_info * options)
		{
			return -ENOTSUP;
		}

		int FuseLink::utimens(const char * path, const struct timespec tv[2])
		{
			return -ENOTSUP;
		}

		int FuseLink::ioctl(const char * path, int cmd, void *arg,
						struct fuse_file_info *, unsigned int flags, void *data)
		{
			return -ENOTSUP;
		}

		int FuseLink::poll(const char * path, struct fuse_file_info * options,
						struct fuse_pollhandle *ph, unsigned *reventsp)
		{
			return -ENOTSUP;
		}

		int Macros::checkPathExists(const char *path)
		{
			return -ENOENT;
		}

		int Macros::checkPathNotExists(const char* path)
		{
			// Assumes the context is file creation, so returns
			// -ENOENT if any of the components of the path
			// except the last one do not exist.  Returns -EEXIST
			// if the entire path exists.  Returns 0 if all except
			// the last component of the path exists.
			return -EEXIST;
		}

		int Macros::checkPathIsValid(const char *path)
		{
			return -EIO;
		}

		int Macros::checkPermission(const char *path, char op, int uid, int gid)
		{
			return -EACCES;
		}
	}
}
