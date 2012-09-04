# vim: set ts=4 sw=4 tw=0 et ai:

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "libapp/lowlevel/util.h" namespace "AppLib::LowLevel::Util":
    bool createPackage(string path, char* appname, char* appver,
            char* appdesc, char* appauthor)
    int translateOpenMode(string mode) except +

cdef extern from "libapp/fsfile.h" namespace "AppLib":
    cdef cppclass FSFile:
        #FSFile(LowLevel::FS* filesystem, LowLevel::BlockStream* fd, uint16_t inodeid)
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
#        int getattr(string path, struct stat* stbuf)
#        int readlink(string path, string& out)
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

#def create_package(path not None, appname not None, appver not None,
#        appdesc not None, appauthor not None):
#    c_create_package(path, appname, appver, appdesc, appauthor)

cdef class Package:
    cdef FS* thisptr
    def __cinit__(self, char* path):
        self.thisptr = new FS(string(path))
    def __dealloc__(self):
        del self.thisptr
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
    
