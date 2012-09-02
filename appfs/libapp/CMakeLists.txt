# vim: set ts=4 sw=4 tw=0 ai et:
include_directories(${APPTOOLS_SOURCE_DIR}/appfs)
add_library(app STATIC
    endian.cpp
    fs.cpp
    fsfile.cpp
    environment.cpp
    freelist.cpp
    fuselink.cpp
    logging.cpp
    util.cpp
    blockstream.cpp
    )
find_package(FUSE REQUIRED)
add_definitions(-D_FILE_OFFSET_BITS=64)
include_directories(${FUSE_INCLUDE_DIRS})
target_link_libraries(app ${FUSE_LIBRARIES})
