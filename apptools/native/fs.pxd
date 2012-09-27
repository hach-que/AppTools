# vim: set ts=4 sw=4 tw=0 et ai syntax=pyrex:

from fsfile cimport FSFile
from system cimport stat
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "libapp/lowlevel/util.h" namespace "AppLib::LowLevel::Util":
    bool createPackage(string path, char* appname, char* appver,
            char* appdesc, char* appauthor)
    int translateOpenMode(string mode) except +

cdef extern from "libapp/fs.h" namespace "AppLib":
    cdef cppclass FS:
        FS(string path) except +
        FS(string path, int uid, int gid) except +
        void getattr(string path, stat stbuf) except +
        string readlink(string path) except +
        void mknod(string path, int mode, int devid) except +
        void mkdir(string path, int mode) except +
        void unlink(string path) except +
        void rmdir(string path) except +
        void symlink(string linkPath, string targetPath) except +
        void rename(string srcPath, string destPath) except +
        void link(string linkPath, string targetPath) except +
        void chmod(string path, int mode) except +
        void chown(string path, int uid, int gid) except +
        void truncate(string path, unsigned long size) except +
        FSFile open(string path) # exceptions not handled; use c_open instead.
        vector[string] readdir(string path) except +
        void create(string path, int mode) except +
        void utimens(string path, int access, int modification) except +
        void setuid(int uid) except +
        void setgid(int gid) except +
        void touch(string path, string modes) except +

cdef class Package:
    cdef FS* thisptr

    cdef FSFile* c_open(self, char* path) except +
