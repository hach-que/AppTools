# vim: set ts=4 sw=4 tw=0 ai et:
add_executable(apputil apputil.cpp)
find_package(PythonLibs REQUIRED)
find_package(PythonInterp REQUIRED)
target_link_libraries(apputil app argtable2 ${PYTHON_LIBRARIES})
include_directories(${APPTOOLS_SOURCE_DIR}/appfs ${PYTHON_INCLUDE_DIRS})
include_directories(${APPTOOLS_SOURCE_DIR}/third-party/argtable2)
add_definitions("-D_FILE_OFFSET_BITS=64")
exec_program("${PYTHON_EXECUTABLE}"
    "${APPTOOLS_SOURCE_DIR}"
    ARGS -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LINKFORSHARED')\""
    OUTPUT_VARIABLE LINKFORSHARED)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKFORSHARED}")
message("-- Using Python link options: ${LINKFORSHARED}")
if("${PYTHON_LIBRARIES}" MATCHES "(.*)\\.so")
    message("!! WARNING: Python is being linked dynamically.")
    message("!! WARNING: For greatest portability, link statically.")
endif()
