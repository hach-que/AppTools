/*

Header file for FS.

This class reads AppFS packages and their contents.  The FuseLink
class uses this class to read and write data.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                22nd June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#ifndef CLASS_FS
#define CLASS_FS

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "endian.h"
#include "fsfile.h"

namespace AppLib
{
	namespace LowLevel
	{
		namespace FSResult
		{
			enum FSResult
			{
				E_SUCCESS,
				E_FAILURE_GENERAL,
				E_FAILURE_INVALID_FILENAME,
				E_FAILURE_INVALID_POSITION,
				E_FAILURE_INODE_ALREADY_ASSIGNED,
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
		//       in the actual AppFS packages.  Therefore, you should
		//       not change the values for INT_FREEBLOCK, INT_FILE,
		//       INT_DIRECTORY, INT_SEGMENT or INT_TEMPORARY since
		//       it will break the ability to read existing packages.
		namespace INodeType
		{
			enum INodeType
			{
				INT_FREEBLOCK = 0,
				INT_FILE = 1,
				INT_DIRECTORY = 2,
				INT_SEGMENT = 3,
				INT_TEMPORARY = 4,
				INT_INVALID = 5,
				INT_UNSET = 255
			};
		}

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
				uint32_t seg_len;
				uint32_t seg_next;

				inline INode(uint16_t id,
								char * filename,
								INodeType::INodeType type,
								uint16_t uid,
								uint16_t gid,
								uint16_t mask,
								uint64_t atime,
								uint64_t mtime,
								uint64_t ctime)
				{
					this->inodeid = id;
					for (uint16_t i = 0; i < strlen(filename); i += 1)
						this->filename[i] = filename[i];
					for (uint16_t i = strlen(filename); i < 256; i += 1)
						this->filename[i] = '\0';
					this->type = type;
					this->uid = uid;
					this->gid = gid;
					this->mask = mask;
					this->atime = atime;
					this->mtime = mtime;
					this->ctime = ctime;
					this->dat_len = 0;
					this->seg_len = 0;
					this->seg_next = 0;
					this->parent = 0;
					this->children_count = 0;
					this->dev = 0;
					this->rdev = 0;
					this->nlink = 1; // we only have one reference to this object
					this->blocks = 0;
					for (uint16_t i = 0; i < DIRECTORY_CHILDREN_MAX; i += 1)
						this->children[i] = 0;
				}

				inline INode(uint16_t id,
								char * filename,
								INodeType::INodeType type)
				{
					this->inodeid = id;
					for (uint16_t i = 0; i < strlen(filename); i += 1)
						this->filename[i] = filename[i];
					for (uint16_t i = strlen(filename); i < 256; i += 1)
						this->filename[i] = '\0';
					this->type = type;
					this->uid = 0;
					this->gid = 0;
					this->mask = 0;
					this->atime = 0;
					this->mtime = 0;
					this->ctime = 0;
					this->dat_len = 0;
					this->seg_len = 0;
					this->seg_next = 0;
					this->parent = 0;
					this->children_count = 0;
					this->dev = 0;
					this->rdev = 0;
					this->nlink = 1; // we only have one reference to this object
					this->blocks = 0;
					for (uint16_t i = 0; i < DIRECTORY_CHILDREN_MAX; i += 1)
						this->children[i] = 0;
				}

				inline std::string getBinaryRepresentation()
				{
					std::stringstream binary_rep;
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->inodeid),  2);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->type),     2);
					if (this->type == INodeType::INT_SEGMENT)
					{
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->seg_len),  4);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->seg_next), 4);
						return binary_rep.str();
					}
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->filename), 256);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->uid),      2);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->gid),      2);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->mask),     2);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->atime),    8);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->mtime),    8);
					Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->ctime),    8);
					if (this->type == INodeType::INT_FILE)
					{
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->dev),      2);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->rdev),     2);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->nlink),    2);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->blocks),   2);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->dat_len),  4);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->seg_len),  4);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->seg_next), 4);
					}
					else
					{
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->parent),   2);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->children_count), 2);
						Endian::doW(&binary_rep, reinterpret_cast<char *>(&this->children), DIRECTORY_CHILDREN_MAX * 2);
					}
					return binary_rep.str();
				}

				inline ~INode()
				{
					// Do nothing.. no cleanup to be done.
				}
		};

		class FS
		{
			public:
				FS(std::fstream * fd);

				// Returns whether the file descriptor is valid.  If this
				// is false, and you call one of the functions in the class
				// (other than getTemporaryNode or isValid), the program will
				// use assert() to ensure the safety of the contents of the
				// package.
				bool isValid();

				// Writes an INode to the specified position and then
				// updates the inode lookup table.
				FSResult::FSResult writeINode(uint32_t pos, INode node);

				// Retrieves an INode by an ID.
				INode getINodeByID(uint16_t id);

				// Retrieves an INode by position.
				INode getINodeByPosition(uint32_t pos);

				// A return value of 0 indicates that the specified INode
				// does not exist.
				uint32_t getINodePositionByID(uint16_t id);

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

				// Adds a child inode to a parent (directory) inode.  Please note that it doesn't
				// check to see whether or not the child is already attached to the parent, but
				// it will add the child reference in the lowest available slot.
				FSResult::FSResult addChildToDirectoryInode(uint16_t parentid, uint16_t childid);

				// Removes a child inode from a parent (directory) inode.
				FSResult::FSResult removeChildFromDirectoryInode(uint16_t parentid, uint16_t childid);

				// Returns whether or not a specified filename is unique
				// inside a directory.  E_SUCCESS indicates unique, E_FAILURE_NOT_UNIQUE
				// indicates not unique.
				FSResult::FSResult filenameIsUnique(uint16_t parentid, char * filename);

				// Returns an std::vector<INode> list of children within
				// the specified directory.
				std::vector<INode> getChildrenOfDirectory(uint16_t parentid);

				// Returns an INode for the child with the specified
				// INode id (or filename) within the specified directory.  Returns an
				// inode with type INodeType::INT_INVALID if it is unable to find
				// the specified child, or if the parent inode is invalid (i.e. not
				// a directory).
				INode getChildOfDirectory(uint16_t parentid, uint16_t childid);
				INode getChildOfDirectory(uint16_t parentid, const char * filename);

				// Sets a file's contents (replacing the current contents).
				FSResult::FSResult setFileContents(uint16_t id, const char * data, uint32_t len);

				// Returns a file's contents.
				FSResult::FSResult getFileContents(uint16_t id, char ** out, uint32_t * len_out, uint32_t len_max);

				// Sets the length of a file (the dat_len and seg_len) fields, without actually
				// adjusting the length of the file data or allocating new blocks (it only changes
				// the field values).
				FSResult::FSResult setFileLengthDirect(uint32_t pos, uint32_t len);

				// Sets the seg_next field for a FILE or SEGMENT block, without actually
				// allocating a new block or validating the seg_next position.
				FSResult::FSResult setFileNextSegmentDirect(uint32_t pos, uint32_t seg_next);

				// This function returns the position of the next block for file data after the current block.
				uint32_t getFileNextBlock(uint32_t pos);

				// Erase a specified block (CAUTION:  This simply erases BSIZE_FILE bytes from the
				// specified position.  It does not check to make sure the position is actually
				// the start of a block!)
				FSResult::FSResult resetBlock(uint32_t pos);

				// Resolves a position in a file to a position in the disk image.
				uint32_t resolvePositionInFile(uint16_t inodeid, uint32_t pos);

				// Resolve a pathname into an inode id.
				int32_t resolvePathnameToInodeID(const char * path);

				// Sets the length of a file, allocating or erasing blocks / data where necessary.
				FSResult::FSResult truncateFile(uint16_t inodeid, uint32_t len);

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
				std::vector<std::string> splitPathBySeperators(std::string path);

			private:
				std::fstream * fd;
		};
	}
}

#endif