# vim: set ts=4 sw=4 tw=0 ai et:
include_directories(${APPTOOLS_SOURCE_DIR}/appfs)
add_library(app STATIC
    lowlevel/endian.cpp
    lowlevel/fs.cpp
    lowlevel/freelist.cpp
    lowlevel/blockstream.cpp
    lowlevel/util.cpp
    internal/fuselink.cpp
    exception/package.cpp
    environment.cpp
    logging.cpp
    fsfile.cpp
    fs.cpp
    )
find_package(FUSE REQUIRED)
add_definitions(-D_FILE_OFFSET_BITS=64)
include_directories(${FUSE_INCLUDE_DIRS})
target_link_libraries(app ${FUSE_LIBRARIES})
add_subdirectory(python)
