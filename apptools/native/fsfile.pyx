# vim: set ts=4 sw=4 tw=0 et ai syntax=pyrex:

#cimport fsfile
from fs cimport Package
#from fs import Package
from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "libapp/lowlevel/util.h" namespace "AppLib::LowLevel::Util":
    int translateOpenMode(string mode) except +

cdef class PackageFile:
    def __cinit__(self, Package fs, char* path, char* mode):
        self.thisptr = fs.c_open(path)
        if (self.thisptr == NULL):
           raise IOError("The specified file does not exist in the package.")
        self.thisptr.open(translateOpenMode(string(mode)))
        # FIXME: Handle exceptions.

    def __dealloc__(self):
        del self.thisptr
    
