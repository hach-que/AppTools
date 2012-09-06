# vim: set ts=4 sw=4 tw=0 et ai:

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "time.h":
    ctypedef unsigned int time_t

cdef extern from "sys/stat.h":
    cdef struct stat:
        # FIXME: Is this portable?
        int st_dev
        int st_ino
        int st_mode
        int st_nlink
        int st_uid
        int st_gid
        int st_rdev
        int st_size
        int st_blksize
        int st_blocks
        # FIXME: These values are not represented correctly!
        time_t st_atime
        time_t st_mtime
        time_t st_ctime

cdef extern from "libapp/lowlevel/util.h" namespace "AppLib::LowLevel::Util":
    bool createPackage(string path, char* appname, char* appver,
            char* appdesc, char* appauthor)
    int translateOpenMode(string mode) except +

cdef extern from "libapp/fsfile.h" namespace "AppLib":
    cdef cppclass FSFile:
        void open(int mode)
        unsigned long read(char* out, int count)
        bool truncate(unsigned long len)
        void close()
        void seekp(unsigned long pos)
        void seekg(unsigned long pos)
        unsigned long tellp()
        unsigned long tellg()
        unsigned int size()

cdef extern from "libapp/fs.h" namespace "AppLib":

    cdef cppclass FS:
        FS(string path)
        void getattr(string path, stat stbuf) except +
        string readlink(string path) except +
#        int mknod(string path, mode_t mask, dev_t devid)
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

def create_package(char* path, char* appname, char* appver,
        char* appdesc, char* appauthor):
    createPackage(string(path), appname, appver, appdesc, appauthor)

class FileStat:
    pass

cdef class Package:
    cdef FS* thisptr

    def __cinit__(self, char* path):
        self.thisptr = new FS(string(path))

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

    def open(self, char* path, char* mode):
        return PackageFile(self, path, mode)

    cdef FSFile* c_open(self, char* path):
        return self.thisptr.open(string(path))

cdef class PackageFile:
    cdef FSFile* thisptr

    def __cinit__(self, Package fs, char* path, char* mode):
        self.thisptr = fs.c_open(path)
        if (self.thisptr == NULL):
            raise IOError("The specified file does not exist in the package.")
        self.thisptr.open(translateOpenMode(string(mode)))
        # FIXME: Handle exceptions.

    def __dealloc__(self):
        del self.thisptr
    
