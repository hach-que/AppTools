# vim: set ts=4 sw=4 tw=0 ai et:
project(APPTOOLS_PYTHON)

# Set up Python copy target.
file(WRITE ${CMAKE_BINARY_DIR}/PythonCopy.cmake
"
cmake_minimum_required(VERSION 2.8.5)
file(COPY ${APPTOOLS_PYTHON_SOURCE_DIR}/
     DESTINATION ${APPTOOLS_PYTHON_BINARY_DIR}
     PATTERN \"native/*\" EXCLUDE
     PATTERN \"CMakeLists.txt\" EXCLUDE
     PATTERN \"*\")
file(COPY ${APPTOOLS_PYTHON_SOURCE_DIR}/native/__init__.py
     DESTINATION ${APPTOOLS_PYTHON_BINARY_DIR}/native/)
")
add_custom_target(apptools ALL COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/PythonCopy.cmake)

add_subdirectory(native)
