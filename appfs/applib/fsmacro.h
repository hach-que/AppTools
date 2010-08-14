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

// We need 64-bit time support.  Since it seems time.h has no
// _time64 function in UNIX,  we are probably going to have to
// statically reference it from an external library.  This macro
// ensures that all of the code will switch over to use the new
// function when such a function is located.  For now, it uses
// time() in time.h.
#define APPFS_TIME() time(NULL);

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
#define APPFS_RETRIEVE_PATH_TO_INODE(nde) \
AppLib::LowLevel::INode nde(0, "", AppLib::LowLevel::INodeType::INT_UNSET); \
int appfs_retrieve_path_to_inode = AppLib::FUSE::Macros::retrievePathToINode(path, &nde); \
if (appfs_retrieve_path_to_inode != 0) return appfs_retrieve_path_to_inode;
#define APPFS_RETRIEVE_PARENT_PATH_TO_INODE(nde) \
AppLib::LowLevel::INode nde(0, "", AppLib::LowLevel::INodeType::INT_UNSET); \
int appfs_retrieve_parent_path_to_inode = AppLib::FUSE::Macros::retrieveParentPathToINode(path, &nde); \
if (appfs_retrieve_parent_path_to_inode != 0) return appfs_retrieve_parent_path_to_inode;
#define APPFS_ASSIGN_NEW_INODE(nde, type) \
AppLib::LowLevel::INode nde(0, "", type); \
int appfs_assign_new_inode = AppLib::FUSE::Macros::assignNewINode(&nde,type); \
if (appfs_assign_new_inode != 0) return appfs_assign_new_inode;
#define APPFS_SAVE_INODE(buf) \
int appfs_save_inode = AppLib::FUSE::Macros::saveINode(&buf); \
if (appfs_save_inode != 0) return appfs_save_inode;
#define APPFS_COPY_CONST_CHAR_TO_FILENAME(ret, filename) \
const char* ret_cpy = ret; /* ret may be the result of a function. */ \
for (int i = 0; i < 256; i += 1) \
	filename[i] = 0; \
for (int i = 0; i < 256; i += 1) \
{ \
	if (ret_cpy[i] == 0) \
		break; \
	filename[i] = ret_cpy[i]; \
}
#ifdef WIN32
#pragma message ( "warning: The APPFS_COPY_INODE_TO_POINTER macro does not support FSINFO blocks." )
#else
#warning "The APPFS_COPY_INODE_TO_POINTER macro does not support FSINFO blocks."
#endif
#define APPFS_COPY_INODE_TO_POINTER(node,pointer) \
pointer->inodeid = node.inodeid; \
for (int i = 0; i < 256; i += 1) \
{ \
pointer->filename[i] = node.filename[i]; \
} \
pointer->type = node.type; \
pointer->uid = node.uid; \
pointer->gid = node.gid; \
pointer->mask = node.mask; \
pointer->atime = node.atime; \
pointer->mtime = node.mtime; \
pointer->ctime = node.ctime; \
pointer->parent = node.parent; \
for (int i = 0; i < node.children_count; i += 1) \
{ \
pointer->children[i] = node.children[i]; \
} \
pointer->children_count = node.children_count; \
pointer->dev = node.dev; \
pointer->rdev = node.rdev; \
pointer->nlink = node.nlink; \
pointer->blocks = node.blocks; \
pointer->dat_len = node.dat_len; \
pointer->info_next = node.info_next; \
pointer->flst_next = node.flst_next;
