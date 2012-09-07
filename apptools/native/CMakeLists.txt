# vim: set ts=4 sw=4 tw=0 ai et:
cmake_minimum_required(VERSION 2.8.5)
project(LIBAPP_PYTHON)
include(UseCython)
add_custom_target(ReplicatePythonSourceTree ALL ${CMAKE_COMMAND} -P
    ${APPTOOLS_SOURCE_DIR}/cmake/ReplicatePythonSourceTree.cmake
    ${APPTOOLS_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${APPTOOLS_SOURCE_DIR})

# Get a list of source files used in the main
# C++ library.
get_target_property(CPP_SOURCE_LIST app SOURCES)
set(PYTHON_SOURCE_LIST "")
foreach(i IN ITEMS ${CPP_SOURCE_LIST})
    set(PYTHON_SOURCE_LIST ${PYTHON_SOURCE_LIST} ${APPTOOLS_SOURCE_DIR}/apptools/libapp/${i})
endforeach()

# Define Cython files.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-writable-strings -Wno-reserved-user-defined-literal")
set_source_files_properties(
    ${LIBAPP_PYTHON_SOURCE_DIR}/fsfile.pyx
    ${LIBAPP_PYTHON_SOURCE_DIR}/fs.pyx
    PROPERTIES CYTHON_IS_CXX TRUE)
cython_add_module(apptools.native fsfile fsfile.pyx)
cython_add_module(apptools.native fs fs.pyx)

add_definitions(-D_FILE_OFFSET_BITS=64)
include_directories(${FUSE_INCLUDE_DIRS})
include_directories(${APPTOOLS_SOURCE_DIR})
target_link_libraries(fsfile app ${FUSE_LIBRARIES})
target_link_libraries(fs app ${FUSE_LIBRARIES})

