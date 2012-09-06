# vim: set ts=4 sw=4 tw=0 et ai syntax=pyrex:

from fsfile cimport PackageFile
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

class FileStat:
    pass

cdef class Package:
    def __cinit__(self, char* path, int uid=0, int gid=0):
        self.thisptr = new FS(string(path), uid, gid)

    def __dealloc__(self):
        del self.thisptr

    def getattr(self, char* path):
        cdef stat value
        self.thisptr.getattr(string(path), value)
        f = FileStat()
        f.dev = value.st_dev
        f.id = value.st_ino
        f.mode = value.st_mode
        f.nlink = value.st_nlink
        f.uid = value.st_uid
        f.gid = value.st_gid
        f.rdev = value.st_rdev
        f.size = value.st_size
        f.blksize = value.st_blksize
        f.blocks = value.st_blocks
        f.atime = value.st_atime
        f.mtime = value.st_mtime
        f.ctime = value.st_ctime
        return f

    def readlink(self, char* path):
        return self.thisptr.readlink(string(path)).c_str()

    def mknod(self, char* path, int mode, int devid):
        self.thisptr.mknod(string(path), mode, devid)

    def open(self, char* path, char* mode):
        return PackageFile(self, path, mode)

    cdef FSFile* c_open(self, char* path):
        return self.thisptr.open(string(path))

