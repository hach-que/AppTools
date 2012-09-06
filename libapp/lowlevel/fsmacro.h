/* vim: set ts=4 sw=4 tw=0 et ai :*/

#define APPFS_MAKE_SYMLINK (fuse_file_info*)0x1000
#define APPFS_CHECK_PATH_EXISTS() \
APPFS_CHECK_PATH_VALIDITY() \
int appfs_check_path_exists_ret = AppLib::FUSE::Macros::checkPathExists(path); \
if (appfs_check_path_exists_ret != 0) return appfs_check_path_exists_ret;
#define APPFS_CHECK_TARGET_EXISTS() \
APPFS_CHECK_TARGET_VALIDITY() \
int appfs_check_target_exists_ret = AppLib::FUSE::Macros::checkPathExists(target); \
if (appfs_check_target_exists_ret != 0) return appfs_check_target_exists_ret;
#define APPFS_CHECK_PATH_NOT_EXISTS() \
APPFS_CHECK_PATH_VALIDITY() \
int appfs_check_path_not_exists_ret = AppLib::FUSE::Macros::checkPathNotExists(path); \
if (appfs_check_path_not_exists_ret != 0) return appfs_check_path_not_exists_ret;
#define APPFS_CHECK_PATH_RENAMABILITY() \
APPFS_CHECK_PATH_VALIDITY() \
int appfs_check_path_renamability_ret = AppLib::FUSE::Macros::checkPathRenamability(path); \
if (appfs_check_path_renamability_ret != 0) return appfs_check_path_renamability_ret;
#define APPFS_CHECK_PATH_VALIDITY() \
int appfs_check_path_is_valid_ret = AppLib::FUSE::Macros::checkPathIsValid(path); \
if (appfs_check_path_is_valid_ret != 0) return appfs_check_path_is_valid_ret;
#define APPFS_CHECK_TARGET_VALIDITY() \
int appfs_check_target_is_valid_ret = AppLib::FUSE::Macros::checkPathIsValid(target); \
if (appfs_check_target_is_valid_ret != 0) return appfs_check_target_is_valid_ret;
#define APPFS_CHECK_PERMISSION(op, uid, gid) \
int appfs_check_path_permission_ret = AppLib::FUSE::Macros::checkPermission(path, op, uid, gid); \
if (appfs_check_path_permission_ret != 0) return appfs_check_path_permission_ret;
#define APPFS_RETRIEVE_PATH_TO_INODE(nde) \
AppLib::LowLevel::INode nde(0, "", AppLib::LowLevel::INodeType::INT_UNSET); \
int appfs_retrieve_path_to_inode = AppLib::FUSE::Macros::retrievePathToINode(path, &nde); \
if (appfs_retrieve_path_to_inode != 0) return appfs_retrieve_path_to_inode;
#define APPFS_RETRIEVE_TARGET_TO_INODE(nde) \
AppLib::LowLevel::INode nde(0, "", AppLib::LowLevel::INodeType::INT_UNSET); \
int appfs_retrieve_target_to_inode = AppLib::FUSE::Macros::retrievePathToINode(target, &nde); \
if (appfs_retrieve_target_to_inode != 0) return appfs_retrieve_target_to_inode;
#define APPFS_RETRIEVE_PARENT_PATH_TO_INODE(nde) \
AppLib::LowLevel::INode nde(0, "", AppLib::LowLevel::INodeType::INT_UNSET); \
int appfs_retrieve_parent_path_to_inode = AppLib::FUSE::Macros::retrieveParentPathToINode(path, &nde); \
if (appfs_retrieve_parent_path_to_inode != 0) return appfs_retrieve_parent_path_to_inode;
#define APPFS_RETRIEVE_PARENT_TARGET_TO_INODE(nde) \
AppLib::LowLevel::INode nde(0, "", AppLib::LowLevel::INodeType::INT_UNSET); \
int appfs_retrieve_parent_target_to_inode = AppLib::FUSE::Macros::retrieveParentPathToINode(target, &nde); \
if (appfs_retrieve_parent_target_to_inode != 0) return appfs_retrieve_parent_target_to_inode;
#define APPFS_ASSIGN_NEW_INODE(nde, type) \
AppLib::LowLevel::INode nde(0, "", type); \
uint32_t appfs_assign_new_inode_pos; \
int appfs_assign_new_inode = AppLib::FUSE::Macros::assignNewINode(&nde,type,appfs_assign_new_inode_pos); \
if (appfs_assign_new_inode != 0) return appfs_assign_new_inode;
#define APPFS_SAVE_INODE(buf) \
int appfs_save_inode = AppLib::FUSE::Macros::saveINode(&buf); \
if (appfs_save_inode != 0) return appfs_save_inode;
#define APPFS_SAVE_NEW_INODE(buf) \
int appfs_save_new_inode = AppLib::FUSE::Macros::saveNewINode(appfs_assign_new_inode_pos, &buf); \
if (appfs_save_new_inode != 0) return appfs_save_new_inode;
#define APPFS_BASENAME_TO_FILENAME(path, filename) \
const char* appfs_copy_basename = Macros::extractBasenameFromPath(path); \
int appfs_copy_const_char_to_filename = INTERNAL_APPFS_COPY_CONST_CHAR_TO_FILENAME(appfs_copy_basename, filename); \
free((void*)appfs_copy_basename); \
if (appfs_copy_const_char_to_filename != 0) return appfs_copy_const_char_to_filename;
#define APPFS_VERIFY_INODE_POSITION(pos) APPFS_VERIFY_INODE_POSITION_CUSTOM(pos, FSResult::E_FAILURE_INVALID_POSITION);
#define APPFS_VERIFY_INODE_POSITION_CUSTOM(pos, errResult) \
if (pos < OFFSET_DATA || (pos - OFFSET_DATA) % 4096 != 0) \
{ \
    APPFS_VERIFY_INODE_ASSERT_POSITION(); \
    return errResult; \
}

#include <assert.h>
#include <libapp/lowlevel/fs.h>

void APPFS_VERIFY_INODE_ASSERT_POSITION();

// TODO: Not be lazy and actually put these as macros in FuseLink.

inline AppLib::LowLevel::FSResult::FSResult INTERNAL_APPFS_COPY_CONST_CHAR_TO_FILENAME(const char *ret, char filename[256])
{
    if (strlen(ret) >= 256)
        return AppLib::LowLevel::FSResult::E_FAILURE_INVALID_FILENAME;
    for (int i = 0; i < 256; i += 1)
        filename[i] = 0;
    for (int i = 0; i < 255; i += 1)
    {
        if (ret[i] == 0)
            break;
        filename[i] = ret[i];
    }
    return AppLib::LowLevel::FSResult::E_SUCCESS;
}

inline void APPFS_COPY_INODE_TO_POINTER(AppLib::LowLevel::INode & node, AppLib::LowLevel::INode * pointer)
{
    pointer->inodeid = node.inodeid;
    for (int i = 0; i < 255; i += 1)
    {
        pointer->filename[i] = node.filename[i];
    }
    for (int i = 0; i < 255; i += 1)
    {
        pointer->realfilename[i] = node.realfilename[i];
    }
    pointer->type = node.type;
    pointer->uid = node.uid;
    pointer->gid = node.gid;
    pointer->mask = node.mask;
    pointer->atime = node.atime;
    pointer->mtime = node.mtime;
    pointer->ctime = node.ctime;
    pointer->parent = node.parent;
    for (int i = 0; i < node.children_count; i += 1)
    {
        pointer->children[i] = node.children[i];
    }
    pointer->children_count = node.children_count;
    pointer->dev = node.dev;
    pointer->rdev = node.rdev;
    pointer->nlink = node.nlink;
    pointer->blocks = node.blocks;
    pointer->dat_len = node.dat_len;
    pointer->info_next = node.info_next;
    pointer->flst_next = node.flst_next;
    pointer->realid = node.realid;
}