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

enum FSResult
{
	E_SUCCESS,
	E_FAILURE_GENERAL,
	E_FAILURE_INVALID_FILENAME,
	E_FAILURE_INVALID_POSITION,
	E_FAILURE_INODE_ALREADY_ASSIGNED,
	E_FAILURE_NOT_A_DIRECTORY,
	E_FAILURE_MAXIMUM_CHILDREN_REACHED,
	E_FAILURE_UNKNOWN
};

// WARN: The values here are also used to store the types
//       in the actual AppFS packages.  Therefore, you should
//       not change the values for INT_FREEBLOCK, INT_FILE or
//       INT_DIRECTORY since it will break the ability to read
//       existing packages.
enum INodeType
{
	INT_FREEBLOCK = 0,
	INT_FILE = 1,
	INT_DIRECTORY = 2,
	INT_INVALID = 3,
	INT_UNSET = 255
};

class INode
{
	public:
		unsigned int inodeid;
		char filename[256];
		INodeType type;
		unsigned int uid;
		unsigned int gid;
		unsigned int mask;
		unsigned long atime;
		unsigned long mtime;
		unsigned long ctime;
		unsigned int parent;
		unsigned int children[DIRECTORY_CHILDREN_MAX];
		unsigned int children_count;
		unsigned long dat_len;
		unsigned long seg_len;

		inline INode(unsigned int id,
						char * filename,
						INodeType type,
						unsigned int uid,
						unsigned int gid,
						unsigned int mask,
						unsigned long atime,
						unsigned long mtime,
						unsigned long ctime)
		{
			this->inodeid = id;
			for (unsigned int i = 0; i < strlen(filename); i += 1)
				this->filename[i] = filename[i];
			for (unsigned int i = strlen(filename); i < 256; i += 1)
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
			this->parent = 0;
			for (unsigned int i = 0; i < DIRECTORY_CHILDREN_MAX; i += 1)
				this->children[i] = 0;
			this->children_count = 0;
		}

		inline INode(unsigned int id,
						char * filename,
						INodeType type)
		{
			this->inodeid = id;
			for (unsigned int i = 0; i < strlen(filename); i += 1)
				this->filename[i] = filename[i];
			for (unsigned int i = strlen(filename); i < 256; i += 1)
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
			this->parent = 0;
			for (unsigned int i = 0; i < DIRECTORY_CHILDREN_MAX; i += 1)
				this->children[i] = 0;
			this->children_count = 0;
		}

		inline std::string getBinaryRepresentation()
		{
			std::stringstream binary_rep;
			binary_rep.write(reinterpret_cast<char *>(&this->inodeid),  2);
			binary_rep.write(reinterpret_cast<char *>(&this->filename), 256);
			binary_rep.write(reinterpret_cast<char *>(&this->type),     2);
			binary_rep.write(reinterpret_cast<char *>(&this->uid),      2);
			binary_rep.write(reinterpret_cast<char *>(&this->gid),      2);
			binary_rep.write(reinterpret_cast<char *>(&this->mask),     2);
			binary_rep.write(reinterpret_cast<char *>(&this->atime),    4);
			binary_rep.write(reinterpret_cast<char *>(&this->mtime),    4);
			binary_rep.write(reinterpret_cast<char *>(&this->ctime),    4);
			if (this->type == INodeType::INT_FILE)
			{
				binary_rep.write(reinterpret_cast<char *>(&this->dat_len),  4);
				binary_rep.write(reinterpret_cast<char *>(&this->seg_len),  4);
			}
			else
			{
				binary_rep.write(reinterpret_cast<char *>(&this->parent),   2);
				binary_rep.write(reinterpret_cast<char *>(&this->children), DIRECTORY_CHILDREN_MAX * 2);
				binary_rep.write(reinterpret_cast<char *>(&this->children_count), 2);
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

		// Writes an INode to the specified position and then
		// updates the inode lookup table.
		FSResult writeINode(unsigned long pos, INode node);

		// Retrieves an INode by an ID.
		INode getINodeByID(unsigned int id);

		// A return value of 0 indicates that the specified INode
		// does not exist.
		unsigned long getINodePositionByID(unsigned int id);

		// Sets the position of an inode in the inode lookup table.
		FSResult setINodePositionByID(unsigned int id, unsigned long pos);

		// Find first free block and return that position.  The user specifies
		// INT_FILE or INT_DIRECTORY to indicate the number of sequential free
		// blocks to find.  A return value of 0 indicates that the file was either
		// at the maximum filesize, or the function could not otherwise find
		// a free block.
		unsigned long getFirstFreeBlock(INodeType type);

		// Find the first free inode number and return it.  A return value of 0
		// indicates that there are no free inode numbers available (we can use
		// a value of 0 since the root inode will always exist and will always
		// have an ID of 0).
		unsigned int getFirstFreeInodeNumber();

		// Adds a child inode to a parent (directory) inode.  Please note that it doesn't
		// check to see whether or not the child is already attached to the parent, but
		// it will add the child reference in the lowest available slot.
		FSResult addChildToDirectoryInode(unsigned int parentid, unsigned int childid);

		// Closes the filesystem.
		void close();

	private:
		std::fstream * fd;
};

#endif