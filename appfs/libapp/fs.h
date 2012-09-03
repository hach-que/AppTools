/* vim: set ts=4 sw=4 tw=0 et ai :*/

#ifndef CLASS_FS
#define CLASS_FS

#include <libapp/config.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <libapp/endian.h>
#include <libapp/fsfile.h>
#include <libapp/blockstream.h>

namespace AppLib
{
    namespace LowLevel
    {
        class FreeList;

        namespace FSResult
        {
            enum FSResult
            {
                E_SUCCESS,
                E_FAILURE_GENERAL,
                E_FAILURE_INVALID_FILENAME,
                E_FAILURE_INVALID_PATH,
                E_FAILURE_INVALID_POSITION,
                E_FAILURE_INODE_ALREADY_ASSIGNED,
                E_FAILURE_INODE_NOT_ASSIGNED,
                E_FAILURE_INODE_NOT_VALID,
                E_FAILURE_NOT_A_DIRECTORY,
                E_FAILURE_NOT_A_FILE,
                E_FAILURE_NOT_UNIQUE,
                E_FAILURE_NOT_IMPLEMENTED,
                E_FAILURE_MAXIMUM_CHILDREN_REACHED,
                E_FAILURE_PARTIAL_TRUNCATION,
                E_FAILURE_UNKNOWN
            };
        }

        // WARN: The values here are also used to store the types
        //       in the actual AppFS packages.  Therefore you should
        //       not change any values since it will break the
        //       ability to read existing packages.
        namespace INodeType
        {
            enum INodeType
            {
                // Free Block (unused in disk images)
                INT_FREEBLOCK = 0,

                // File Information Block
                INT_FILEINFO = 1,
                // Segment Information Block
                INT_SEGINFO = 2,
                // Data Block (unused in disk images)
                INT_DATA = 254,

                // Directory Block
                INT_DIRECTORY = 3,

                // Symbolic Link
                INT_SYMLINK = 4,
                // Hard Link
                INT_HARDLINK = 5,
                // Special File (FIFO / Device)
                INT_DEVICE = 10,

                // Temporary Block
                INT_TEMPORARY = 6,
                // Free Space Allocation Block
                INT_FREELIST = 7,
                // Filesystem Information Block
                INT_FSINFO = 8,

                // Invalid and Unset Blocks (unused in disk images)
                INT_INVALID = 9,
                INT_UNSET = 255
            };
        }

        class FS;

        class INode
        {
              public:
            uint16_t inodeid;
            char filename[256];
             INodeType::INodeType type;
            uint16_t uid;
            uint16_t gid;
            uint16_t mask;
            uint64_t atime;
            uint64_t mtime;
            uint64_t ctime;
            uint16_t parent;
            uint16_t children[DIRECTORY_CHILDREN_MAX];
            uint16_t children_count;
            uint16_t dev;
            uint16_t rdev;
            uint16_t nlink;
            uint16_t blocks;
            uint32_t dat_len;
            uint32_t info_next;
            uint32_t flst_next;
            uint16_t realid;
            char realfilename[256]; // In-memory only (never written to disk).

            // FSInfo only
            char fs_name[10];
            uint16_t ver_major;
            uint16_t ver_minor;
            uint16_t ver_revision;
            char app_name[256];
            char app_ver[32];
            char app_desc[1024];
            char app_author[256];
            uint32_t pos_root;
            uint32_t pos_freelist;

            inline INode(uint16_t id, const char *filename, INodeType::INodeType type, uint16_t uid, uint16_t gid, uint16_t mask, uint64_t atime, uint64_t mtime, uint64_t ctime)
            {
                this->inodeid = id;
                this->setFilename(filename, "");
                this->type = type;
                this->uid = uid;
                this->gid = gid;
                this->mask = mask;
                this->atime = atime;
                this->mtime = mtime;
                this->ctime = ctime;
                this->dat_len = 0;
                this->info_next = 0;
                this->flst_next = 0;
                this->realid = 0;
                this->parent = 0;
                this->children_count = 0;
                this->dev = 0;
                this->rdev = 0;
                this->nlink = 1;	// we only have one reference to this object
                this->blocks = 0;
                for (uint16_t i = 0; i < DIRECTORY_CHILDREN_MAX; i += 1)
                    this->children[i] = 0;

                // FSInfo only
                for (uint16_t i = 0; i < 10; i += 1)
                    this->fs_name[i] = FS_NAME[i];
                this->ver_major = 0;
                this->ver_minor = 0;
                this->ver_revision = 0;
                for (uint16_t i = 0; i < 256; i += 1)
                    this->app_name[i] = '\0';
                for (uint16_t i = 0; i < 32; i += 1)
                    this->app_ver[i] = '\0';
                for (uint16_t i = 0; i < 1024; i += 1)
                    this->app_desc[i] = '\0';
                for (uint16_t i = 0; i < 256; i += 1)
                    this->app_author[i] = '\0';
                this->pos_root = 0;
                this->pos_freelist = 0;
            }

            inline INode(uint16_t id, const char *filename, INodeType::INodeType type)
            {
                this->inodeid = id;
                this->setFilename(filename, "");
                this->type = type;
                this->uid = 0;
                this->gid = 0;
                this->mask = 0;
                this->atime = 0;
                this->mtime = 0;
                this->ctime = 0;
                this->dat_len = 0;
                this->info_next = 0;
                this->flst_next = 0;
                this->realid = 0;
                this->parent = 0;
                this->children_count = 0;
                this->dev = 0;
                this->rdev = 0;
                this->nlink = 1;	// we only have one reference to this object
                this->blocks = 0;
                for (uint16_t i = 0; i < DIRECTORY_CHILDREN_MAX; i += 1)
                    this->children[i] = 0;

                // FSInfo only
                for (uint16_t i = 0; i < 10; i += 1)
                    this->fs_name[i] = FS_NAME[i];
                this->ver_major = 0;
                this->ver_minor = 0;
                this->ver_revision = 0;
                for (uint16_t i = 0; i < 256; i += 1)
                    this->app_name[i] = '\0';
                for (uint16_t i = 0; i < 32; i += 1)
                    this->app_ver[i] = '\0';
                for (uint16_t i = 0; i < 1024; i += 1)
                    this->app_desc[i] = '\0';
                for (uint16_t i = 0; i < 256; i += 1)
                    this->app_author[i] = '\0';
                this->pos_root = 0;
                this->pos_freelist = 0;
            }

            inline std::string getBinaryRepresentation()
            {
                std::stringstream binary_rep;
                Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->inodeid), 2);
                Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->type), 2);
                if (this->type == INodeType::INT_SEGINFO)
                {
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->info_next), 4);
                     return binary_rep.str();
                }
                else if (this->type == INodeType::INT_FREELIST)
                {
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->flst_next), 4);
                    return binary_rep.str();
                }
                else if (this->type == INodeType::INT_FSINFO)
                {
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->fs_name), 10);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->ver_major), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->ver_minor), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->ver_revision), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->app_name), 256);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->app_ver), 32);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->app_desc), 1024);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->app_author), 256);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->pos_root), 4);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->pos_freelist), 4);
                    return binary_rep.str();
                }
                if ((this->type == INodeType::INT_FILEINFO || this->type == INodeType::INT_DEVICE) && this->realid != 0)
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->realfilename), 256);
                else
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->filename), 256);

                if (this->type != INodeType::INT_HARDLINK)
                {
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->uid), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->gid), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->mask), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->atime), 8);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->mtime), 8);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->ctime), 8);
                }
                if (this->type == INodeType::INT_FILEINFO || this->type == INodeType::INT_SYMLINK || this->type == INodeType::INT_DEVICE)
                {
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->dev), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->rdev), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->nlink), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->blocks), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->dat_len), 4);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->info_next), 4);
                }
                else if (this->type == INodeType::INT_DIRECTORY)
                {
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->parent), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->children_count), 2);
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->children), DIRECTORY_CHILDREN_MAX * 2);
                }
                else if (this->type == INodeType::INT_HARDLINK)
                    Endian::doW(&binary_rep, reinterpret_cast < char *>(&this->realid), 2);
                return binary_rep.str();
            }

            inline void setFilename(const char *name, const char *real = "")
            {
                uint16_t size = (strlen(name) < 255) ? strlen(name) : 255;
                for (uint16_t i = 0; i < size; i += 1)
                    this->filename[i] = name[i];
                for (uint16_t i = size; i < 256; i += 1)
                    this->filename[i] = '\0';
                uint16_t rsize = (strlen(real) < 255) ? strlen(real) : 255;
                for (uint16_t i = 0; i < rsize; i += 1)
                    this->realfilename[i] = real[i];
                for (uint16_t i = size; i < 256; i += 1)
                    this->realfilename[i] = '\0';
            }

            inline void setAppName(const char *name)
            {
                uint16_t size = (strlen(name) < 255) ? strlen(name) : 255;
                for (uint16_t i = 0; i < size; i += 1)
                    this->app_name[i] = name[i];
                for (uint16_t i = size; i < 256; i += 1)
                    this->app_name[i] = '\0';
            }

            inline void setAppVersion(const char *name)
            {
                uint16_t size = (strlen(name) < 31) ? strlen(name) : 31;
                for (uint16_t i = 0; i < size; i += 1)
                    this->app_ver[i] = name[i];
                for (uint16_t i = size; i < 32; i += 1)
                    this->app_ver[i] = '\0';
            }

            inline void setAppDesc(const char *name)
            {
                uint16_t size = (strlen(name) < 1023) ? strlen(name) : 1023;
                for (uint16_t i = 0; i < size; i += 1)
                    this->app_desc[i] = name[i];
                for (uint16_t i = size; i < 1024; i += 1)
                    this->app_desc[i] = '\0';
            }

            inline void setAppAuthor(const char *name)
            {
                uint16_t size = (strlen(name) < 255) ? strlen(name) : 255;
                for (uint16_t i = 0; i < size; i += 1)
                    this->app_author[i] = name[i];
                for (uint16_t i = size; i < 256; i += 1)
                    this->app_author[i] = '\0';
            }

            // Ensures that the node data is valid.
            inline bool verify()
            {
                if (this->filename[0] == 0 && (this->type == INodeType::INT_DIRECTORY || this->type == INodeType::INT_FILEINFO) && this->inodeid != 0)
                    return false;
                return true;
            }

            inline ~ INode()
            {
                // Do nothing.. no cleanup to be done.
            }

            // Resolves a hardlink to the real file.
            INode resolve(FS* filesystem);
        };

        class FS
        {
              public:
            FS(LowLevel::BlockStream * fd);

            // Returns whether the file descriptor is valid.  If this
            // is false, and you call one of the functions in the class
            // (other than getTemporaryNode or isValid), the program will
            // use assert() to ensure the safety of the contents of the
            // package.
            bool isValid();

            // Writes an INode to the specified position and then
            // updates the inode lookup table.
             FSResult::FSResult writeINode(uint32_t pos, INode node);

            // Updates an INode.  The node must exist in the inode
            // position lookup table.
             FSResult::FSResult updateINode(INode node);

            // Updates a raw INode (such as a freelist block).
             FSResult::FSResult updateRawINode(INode node, uint32_t pos);

            // Retrieves an INode by an ID.
            INode getINodeByID(uint16_t id);

            // Retrieves an INode by an ID, without performing hardlink
            // resolution.
            INode getRealINodeByID(uint16_t id);

            // Retrieves an INode by position.
            INode getINodeByPosition(uint32_t pos);

            // Retrieves the real INode by position (in the case of hardlinks).
            INode getINodeByRealPosition(uint32_t rpos);

            // A return value of 0 indicates that the specified INode
            // does not exist.  Underlyingly calls getInodePositionByID(id, true);
            uint32_t getINodePositionByID(uint16_t id);

            // A return value of 0 indicates that the specified INode
            // does not exist.
            uint32_t getINodePositionByID(uint16_t id, bool resolveHardlinks);

            // Sets the position of an inode in the inode lookup table.
             FSResult::FSResult setINodePositionByID(uint16_t id, uint32_t pos);

            // Find first free block and return that position.  The user specifies
            // INT_FILE or INT_DIRECTORY to indicate the number of sequential free
            // blocks to find.  A return value of 0 indicates that the file was either
            // at the maximum filesize, or the function could not otherwise find
            // a free block.
            uint32_t getFirstFreeBlock(INodeType::INodeType type);

            // Find the first free inode number and return it.  A return value of 0
            // indicates that there are no free inode numbers available (we can use
            // a value of 0 since the root inode will always exist and will always
            // have an ID of 0).
            uint16_t getFirstFreeInodeNumber();

            // Returns whether the specified block is free according to the freelist.
            bool isBlockFree(uint32_t pos);

            // Adds a child inode to a parent (directory) inode.  Please note that it doesn't
            // check to see whether or not the child is already attached to the parent, but
            // it will add the child reference in the lowest available slot.
             FSResult::FSResult addChildToDirectoryInode(uint16_t parentid, uint16_t childid);

            // Removes a child inode from a parent (directory) inode.
             FSResult::FSResult removeChildFromDirectoryInode(uint16_t parentid, uint16_t childid);

            // Returns whether or not a specified filename is unique
            // inside a directory.  E_SUCCESS indicates unique, E_FAILURE_NOT_UNIQUE
            // indicates not unique.
             FSResult::FSResult filenameIsUnique(uint16_t parentid, char *filename);

            // Returns an std::vector<INode> list of children within
            // the specified directory.
             std::vector < INode > getChildrenOfDirectory(uint16_t parentid);

            // Returns an INode for the child with the specified
            // INode id (or filename) within the specified directory.  Returns an
            // inode with type INodeType::INT_INVALID if it is unable to find
            // the specified child, or if the parent inode is invalid (i.e. not
            // a directory).
            INode getChildOfDirectory(uint16_t parentid, uint16_t childid);
            INode getChildOfDirectory(uint16_t parentid, const char *filename);

            // Sets a file's contents (replacing the current contents).
             FSResult::FSResult setFileContents(uint16_t id, const char *data, uint32_t len);

            // Returns a file's contents.
             FSResult::FSResult getFileContents(uint16_t id, char **out, uint32_t * len_out, uint32_t len_max);

            // Sets the length of a file (the dat_len and seg_len) fields, without actually
            // adjusting the length of the file data or allocating new blocks (it only changes
            // the field values).
             FSResult::FSResult setFileLengthDirect(uint32_t pos, uint32_t len);

            // Sets the seg_next field for a FILE or SEGMENT block, without actually
            // allocating a new block or validating the seg_next position.
             FSResult::FSResult setFileNextSegmentDirect(uint16_t id, uint32_t pos, uint32_t seg_next);

            // This function returns the position of the next block for file data after the current block.
            uint32_t getFileNextBlock(uint16_t id, uint32_t pos);

            // Erase a specified block, marking it as free in the free list.
            // (CAUTION:  This simply erases BSIZE_FILE bytes from the specified
            // position.  It does not check to make sure the position is actually
            // the start of a block!)
             FSResult::FSResult resetBlock(uint32_t pos);

            // Resolves a position in a file to a position in the disk image.
            uint32_t resolvePositionInFile(uint16_t inodeid, uint32_t pos);

            // Resolve a pathname into an inode id.
            int32_t resolvePathnameToInodeID(const char *path);

            // Sets the length of a file, allocating or erasing blocks / data where necessary.
             FSResult::FSResult truncateFile(uint16_t inodeid, uint32_t len);

            // Allocates or frees enough blocks so that there is enough segment list blocks
            // available to address all of the segments.  pos is the position of the file
            // inode and len is the length of the data that needs to be addressed (i.e. size
            // of the file).
             FSResult::FSResult allocateInfoListBlocks(uint32_t pos, uint32_t len);

            // Returns a FSFile object for interacting with the specified file at
            // the specified inode.
            FSFile getFile(uint16_t inodeid);

            // Retrieves the temporary block or creates a new one if there is not one currently
            // located within the package (packages should have no more than one temporary block
            // at a single time).  The return value is the position to seek to, where you can then
            // read and write up to BSIZE_FILE - 4 bytes.
            //
            // WARNING: The lookup / creation is *not* fast on large packages as it must search
            //          though all of the inodes in the package.  Therefore, this function makes use
            //          of a static variable to speed up subsequent calls to the function (the first
            //          call must still search through).  If you want to force the function to research
            //          the package for the temporary block (i.e. if you delete the temporary block),
            //          you can use the forceRecheck argument to do so.
            uint32_t getTemporaryBlock(bool forceRecheck = false);

            // Closes the filesystem.
            void close();

            // Utility function for splitting paths into their components.
             std::vector < std::string > splitPathBySeperators(std::string path);

            // Utility function for verifying the validity of a supplied path.
             FSResult::FSResult verifyPath(std::string original, std::vector < std::string > *split);

            // Reserves an INode ID for future use without require the INode to actually be
            // written to disk.
            inline void reserveINodeID(uint16_t id)
            {
                this->reservedINodes.insert(this->reservedINodes.end(), id);
            }

            // Removes an INode ID reservation.
            inline void unreserveINodeID(uint16_t id)
            {
                std::vector < uint16_t >::iterator it = std::find(this->reservedINodes.begin(), this->reservedINodes.end(), id);
                if (it != this->reservedINodes.end())
                    this->reservedINodes.erase(it);
            }

            // Update times.
            inline void updateTimes(uint16_t id, bool atime, bool mtime, bool ctime)
            {
                INode node = this->getINodeByID(id);
                if (atime)
                    node.atime = APPFS_TIME();
                if (mtime)
                    node.mtime = APPFS_TIME();
                if (ctime)
                    node.ctime = APPFS_TIME();
                this->updateINode(node);
            }

              private:
            LowLevel::BlockStream * fd;
            LowLevel::FreeList * freelist;
            std::vector < uint16_t > reservedINodes;
        };
    }
}

#endif
