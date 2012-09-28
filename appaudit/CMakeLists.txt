# vim: set ts=4 sw=4 tw=0 ai et:
include_directories(${APPTOOLS_SOURCE_DIR})
include_directories(${APPTOOLS_SOURCE_DIR}/third-party/argtable2)

find_package(DBus REQUIRED)
add_library(appaudit SHARED audit.c)
target_link_libraries(appaudit ${DBUS_LIBRARIES})
include_directories(${DBUS_INCLUDE_DIR} ${DBUS_ARCH_INCLUDE_DIR})

# Testing binaries.
add_library(audittestlib SHARED testlib.c)
add_executable(audittest test.c)
target_link_libraries(audittest audittestlib)
