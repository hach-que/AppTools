# vim: set ts=4 sw=4 tw=0 et ai syntax=pyrex:

cdef extern from "time.h":
    ctypedef unsigned int time_t

cdef extern from "sys/stat.h":
    cdef struct stat:
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

