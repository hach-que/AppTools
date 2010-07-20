/*

Code file for FuseLink.

This class recieves callbacks from FUSE and handles each operation
by passing them to the actual FS class in a C++ friendly way.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                22nd June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#ifndef WIN32
#include <fuse.h>
#endif
#include <stdio.h>
#include <errno.h>
#include "fuse.h"

#include <string>

namespace AppLib
{
	namespace FUSE
	{
		LowLevel::FS FuseLink::filesystem = NULL;

		extern "C"
		{
			static const struct fuse_operations appfs_ops = {
				.getattr	= FuseLink::getattr,
				.readlink	= FuseLink::readlink,
				.mknod		= FuseLink::mknod,
				.mkdir		= FuseLink::mkdir,
				.unlink		= FuseLink::unlink,
				.rmdir		= FuseLink::rmdir,
				.symlink	= FuseLink::symlink,
				.rename		= FuseLink::rename,
				.link		= FuseLink::link,
				.chmod		= FuseLink::chmod,
				.chown		= FuseLink::chown,
				.truncate	= FuseLink::truncate,
				.open		= FuseLink::open,
				.read		= FuseLink::read,
				.write		= FuseLink::write,
				.statfs		= FuseLink::statfs,
				.flush		= FuseLink::flush,
				.release	= FuseLink::release,
				.fsync		= FuseLink::fsync,
				.setxattr	= NULL
				.getxattr	= NULL
				.listxattr	= NULL
				.removexattr= NULL
				.opendir	= FuseLink::opendir,
				.readdir	= FuseLink::readdir,
				.releasedir	= FuseLink::releasedir,
				.fsyncdir	= FuseLink::fsyncdir,
				.init		= FuseLink::init,
				.destroy	= FuseLink::destroy,
				.access		= FuseLink::access,
				.create		= FuseLink::create,
				.ftruncate	= FuseLink::ftruncate,
				.fgetattr	= FuseLink::fgetattr,
				.lock		= NULL,
				.utimens	= FuseLink::utimens,
				.bmap		= NULL
			}
		}

		Mounter::Mounter(const char *disk_image,
						const char *mount_path,
						bool foreground,
						void (*continue_func)(void))
		{
			// Mounts the specified disk image at the
			// specified mount path using FUSE.
			struct fuse_args fargs = FUSE_ARGS_INIT(0, NULL);

			fuse_main(fargs.argc, fargs.argv, &appfs_ops);
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

#if 0 == 1
		int FuseLink::readlink(const char * path, char * out, size_t size)
		{
		}

		int FuseLink::mknod(const char * path, mode_t mask, dev_t devid);
		int FuseLink::mkdir(const char * path, mode_t mask);
		int FuseLink::unlink(const char * path);
		int FuseLink::rmdir(const char * path);
		int FuseLink::symlink(const char * path, const char * source);
		int FuseLink::rename(const char * path, const char * dest);
		int FuseLink::link(const char * path, const char * source);
		int FuseLink::chmod(const char * path, mode_t mode);
		int FuseLink::chown(const char * path, uid_t user, gid_t group);
		int FuseLink::truncate(const char * path, off_t size);
		int FuseLink::open(const char * path, struct fuse_file_info * options);
		int FuseLink::read(const char * path, char * out, size_t length, off_t offset,
						struct fuse_file_info * options);
		int FuseLink::write(const char *, const char *, size_t, off_t,
						struct fuse_file_info *);
		int FuseLink::statfs(const char *, struct statvfs *);
		int FuseLink::flush(const char *, struct fuse_file_info *);
		int FuseLink::release(const char *, struct fuse_file_info *);
		int FuseLink::fsync(const char *, int, struct fuse_file_info *);
		int FuseLink::setxattr(const char *, const char *, const char *, size_t, int);
		int FuseLink::getxattr(const char *, const char *, char *, size_t);
		int FuseLink::listxattr(const char *, char *, size_t);
		int FuseLink::removexattr(const char *, const char *);
		int FuseLink::opendir(const char *, struct fuse_file_info *);
		int FuseLink::readdir(const char *, void *, fuse_fill_dir_t, off_t,
						struct fuse_file_info *);
		int FuseLink::releasedir(const char *, struct fuse_file_info *);
		int FuseLink::fsyncdir(const char *, int, struct fuse_file_info *);
		void FuseLink::init(struct fuse_conn_info *conn);
		void FuseLink::destroy(void *);
		int FuseLink::access(const char *, int);
		int FuseLink::create(const char *, mode_t, struct fuse_file_info *);
		int FuseLink::ftruncate(const char *, off_t, struct fuse_file_info *);
		int FuseLink::fgetattr(const char *, struct stat *, struct fuse_file_info *);
		int FuseLink::lock(const char *, struct fuse_file_info *, int cmd,
						struct flock *);
		int FuseLink::utimens(const char *, const struct timespec tv[2]);
		int FuseLink::bmap(const char *, size_t blocksize, uint64_t *idx);
		unsigned int FuseLink::flag_nullpath_ok : 1;
		unsigned int FuseLink::flag_reserved : 31;
		int FuseLink::ioctl(const char *, int cmd, void *arg,
						struct fuse_file_info *, unsigned int flags, void *data);
		int FuseLink::poll(const char *, struct fuse_file_info *,
						struct fuse_pollhandle *ph, unsigned *reventsp);
#endif
	}
}
