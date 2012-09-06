# vim: set ts=4 sw=4 tw=0 et ai syntax=pyrex:

from fsfile cimport FSFile
from system cimport stat
from libcpp.string cimport string

cdef extern from "libapp/fs.h" namespace "AppLib":
    cdef cppclass FS:
        FS(string path) except +
        FS(string path, int uid, int gid) except +
        void getattr(string path, stat stbuf) except +
        string readlink(string path) except +
        void mknod(string path, int mask, int devid) except +
#        int mkdir(string path, mode_t mask)
#        int unlink(string path)
#        int rmdir(string path)
#        int symlink(string linkPath, string targetPath)
#        int rename(string srcPath, string destPath)
#        int link(string linkPath, string targetPath)
#        int chmod(string path, mode_t mask)
#        int chown(string path, uid_t user, gid_t group)
#        int truncate(string path, off_t size)
        FSFile* open(string path)
#        int statfs(string path, struct statvfs* stbuf)

cdef class Package:
    cdef FS* thisptr

    cdef FSFile* c_open(self, char* path)
