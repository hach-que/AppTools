# vim: set ts=4 sw=4 tw=0 et ai syntax=pyrex:

from libcpp cimport bool

cdef extern from "libapp/fsfile.h" namespace "AppLib":
    cdef cppclass FSFile:
        FSFile(FSFile copy)
        void open(int mode)
        unsigned long read(char* out, int count)
        bool truncate(unsigned long len)
        void close()
        void seekp(unsigned long pos)
        void seekg(unsigned long pos)
        unsigned long tellp()
        unsigned long tellg()
        unsigned int size()

cdef class PackageFile:
    cdef FSFile* thisptr
