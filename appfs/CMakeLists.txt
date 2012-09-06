# vim: set ts=4 sw=4 tw=0 ai et:
include_directories(${APPTOOLS_SOURCE_DIR})
include_directories(${APPTOOLS_SOURCE_DIR}/third-party/argtable2)
#link_directories(${APPTOOLS_BINARY_DIR}/appfs/libapp)

add_executable(appfs appfs.cpp)
add_executable(appmount appmount.cpp)
add_executable(appcreate appcreate.cpp)
add_executable(appinspect appinspect.cpp)
target_link_libraries(appfs app argtable2 pthread)
target_link_libraries(appmount app argtable2)
target_link_libraries(appcreate app argtable2)
target_link_libraries(appinspect app argtable2)
add_definitions("-D_FILE_OFFSET_BITS=64")
