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

    def mkdir(self, char* path, int mode):
        self.thisptr.mkdir(string(path), mode)

    def unlink(self, char* path):
        self.thisptr.unlink(string(path))

    def rmdir(self, char* path):
        self.thisptr.rmdir(string(path))

    def symlink(self, char* linkPath, char* targetPath):
        self.thisptr.symlink(string(linkPath), string(targetPath))

    def rename(self, char* srcPath, char* destPath):
        self.thisptr.rename(string(srcPath), string(destPath))

    def link(self, char* linkPath, char* targetPath):
        self.thisptr.link(string(linkPath), string(targetPath))

    def chmod(self, char* path, int mode):
        self.thisptr.chmod(string(path), mode)

    def chown(self, char* path, int uid, int gid):
        self.thisptr.chown(string(path), uid, gid)

    def truncate(self, char* path, unsigned long size):
        self.thisptr.truncate(string(path), size)

    def open(self, char* path, char* mode):
        return PackageFile(self, path, mode)

    def readdir(self, char* path):
        cdef vector[string] cresult
        cdef int i
        cresult = self.thisptr.readdir(string(path))
        result = list()
        for i in range(cresult.size()):
            result.append(cresult[i].c_str())
        return result

    def create(self, char* path, int mode):
        self.thisptr.create(string(path), mode)

    def utimens(self, char* path, int access, int modification):
        self.thisptr.utimens(string(path), access, modification)

    def setuid(self, int uid):
        self.thisptr.setuid(uid)

    def setgid(self, int gid):
        self.thisptr.setgid(gid)

    def touch(self, char* path, char* modes):
        self.thisptr.touch(string(path), string(modes))

    cdef FSFile* c_open(self, char* path):
        return new FSFile(self.thisptr.open(string(path)))

