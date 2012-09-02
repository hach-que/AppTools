# vim: set ts=4 sw=4 tw=0 ai et:
cmake_minimum_required(VERSION 2.8)
project(APPTOOLS)
set(CMAKE_MODULE_PATH ${APPTOOLS_SOURCE_DIR}/cmake)

add_subdirectory(third-party)
add_subdirectory(appfs)

