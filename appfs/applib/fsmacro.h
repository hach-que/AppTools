/*

Header file for FS macros.

These macros are used in the FUSE functions to perform common
checks on the arguments provided to the function (such as
file existance, etc.)  The macros are not seperate C++ functions
for ease-of-use within FUSE callbacks.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#define APPFS_CHECK_PATH_EXISTS() \
int appfs_check_path_exists_ret = AppLib::FUSE::Macros::checkPathExists(path); \
if (appfs_check_path_exists_ret != 0) return appfs_check_path_exists_ret;
#define APPFS_CHECK_PATH_NOT_EXISTS() \
int appfs_check_path_not_exists_ret = AppLib::FUSE::Macros::checkPathNotExists(path); \
if (appfs_check_path_not_exists_ret != 0) return appfs_check_path_not_exists_ret;
#define APPFS_CHECK_PATH_VALIDITY() \
int appfs_check_path_is_valid_ret = AppLib::FUSE::Macros::checkPathIsValid(path); \
if (appfs_check_path_is_valid_ret != 0) return appfs_check_path_is_valid_ret;
#define APPFS_CHECK_PERMISSION(op, uid, gid) \
int appfs_check_path_permission_ret = AppLib::FUSE::Macros::checkPermission(path, op, uid, gid); \
if (appfs_check_path_permission_ret != 0) return appfs_check_path_permission_ret;