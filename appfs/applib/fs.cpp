/*

Code file for FS.

This class reads AppFS packages and their contents.  The FuseLink
class uses this class to read and write data.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                22nd June 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"

#include <string>
#include <iostream>
#include <fstream>
#include "fs.h"
#include "logging.h"

FS::FS(std::fstream * fd)
{
	if (fd == NULL)
		Logging::showInternalW("NULL file descriptor passed to FS constructor.");

	this->fd = fd;
}

INode FS::getINodeByID(unsigned int id)
{
	// Retrieve the position using our getINodePositionByID
	// function.
	unsigned long ipos = this->getINodePositionByID(id);
	if (ipos == 0 || ipos < OFFSET_ROOTINODE)
		return INode(0, "", INodeType::INT_INVALID);
	INode node(0, "", INodeType::INT_INVALID);

	// Seek to the inode position
	std::streampos old = this->fd->tellg();
	this->fd->seekg(ipos);

	// Read the data.
	this->fd->read(reinterpret_cast<char *>(&node.inodeid),  2);
	this->fd->read(reinterpret_cast<char *>(&node.filename), 256);
	this->fd->read(reinterpret_cast<char *>(&node.type),     2);
	this->fd->read(reinterpret_cast<char *>(&node.uid),      2);
	this->fd->read(reinterpret_cast<char *>(&node.gid),      2);
	this->fd->read(reinterpret_cast<char *>(&node.mask),     2);
	this->fd->read(reinterpret_cast<char *>(&node.atime),    4);
	this->fd->read(reinterpret_cast<char *>(&node.mtime),    4);
	this->fd->read(reinterpret_cast<char *>(&node.ctime),    4);
	if (node.type == INodeType::INT_FILE)
	{
		this->fd->read(reinterpret_cast<char *>(&node.dat_len),  4);
		this->fd->read(reinterpret_cast<char *>(&node.seg_len),  4);
	}
	else if (node.type == INodeType::INT_DIRECTORY)
	{
		this->fd->read(reinterpret_cast<char *>(&node.parent),   2);
		this->fd->read(reinterpret_cast<char *>(&node.children), DIRECTORY_CHILDREN_MAX * 2);
	}

	// Seek back to the original reading position.
	this->fd->seekg(old);

	return node;
}

FSResult FS::writeINode(unsigned long pos, INode node)
{
	// Check to make sure the position is valid.
	if (pos < OFFSET_DATA)
		return FSResult::E_FAILURE_INVALID_POSITION;

	// Check to make sure the inode ID is not already assigned.
	if (this->getINodePositionByID(node.inodeid) != 0)
		return FSResult::E_FAILURE_INODE_ALREADY_ASSIGNED;

	std::streampos old = this->fd->tellp();
	std::string data = node.getBinaryRepresentation();
	this->fd->seekp(pos);
	this->fd->write(data.c_str(), data.length());
	const char* z = ""; // a const char* always has a \0 terminator, which we use to write into the file.
	if (node.type == INodeType::INT_FILE)
	{
		for (int i = 0; i < BSIZE_FILE - data.length(); i += 1)
			this->fd->write(z, 1);
	}
	else if (node.type == INodeType::INT_DIRECTORY)
	{
		for (int i = 0; i < BSIZE_DIRECTORY - data.length(); i += 1)
			this->fd->write(z, 1);
	}
	this->setINodePositionByID(node.inodeid, pos);
	this->fd->seekp(old);
	return FSResult::E_SUCCESS;
}

unsigned long FS::getINodePositionByID(unsigned int id)
{
	std::streampos old = this->fd->tellg();
	this->fd->seekg(OFFSET_LOOKUP + (id * 4));
	unsigned long ipos = 0;
	this->fd->read(reinterpret_cast<char *>(&ipos), 4);
	this->fd->seekg(old);
	return ipos;
}

unsigned int FS::getFirstFreeInodeNumber()
{
	std::streampos old = this->fd->tellg();
	this->fd->seekg(OFFSET_LOOKUP);
	unsigned long ipos = 0;
	unsigned int count = 0;
	unsigned int ret = 0;
	this->fd->read(reinterpret_cast<char *>(&ipos), 4);
	while (ipos != 0 && count < 65535)
	{
		this->fd->read(reinterpret_cast<char *>(&ipos), 4);
		count += 1;
	}
	if (count == 65535 && ipos != 0)
		ret = 0;
	else
		ret = count;
	this->fd->seekg(old);
	return ret;
}

FSResult FS::setINodePositionByID(unsigned int id, unsigned long pos)
{
	std::streampos old = this->fd->tellp();
	this->fd->seekp(OFFSET_LOOKUP + (id * 4));
	this->fd->write(reinterpret_cast<char *>(&pos), 4);
	this->fd->seekp(old);
	return FSResult::E_SUCCESS;
}

unsigned long FS::getFirstFreeBlock(INodeType type)
{
	std::streampos old = this->fd->tellg();

	// Since the inode number can be 0, and the filename can be blank, we
	// check the value of the type field (offset 258).  If it's 0, then
	// it's a free block.
	signed int type_offset = 258;
	unsigned long current_pos = OFFSET_DATA + type_offset;
	unsigned int current_type = (unsigned int)INodeType::INT_UNSET;
	this->fd->seekg(current_pos);
	this->fd->read(reinterpret_cast<char *>(&current_type), 2);
	while (!this->fd->eof())
	{
		if (current_type == (unsigned int)INodeType::INT_UNSET)
		{
			// Error reading.
			Logging::showErrorW("Unable to read block at position %i.", current_pos - type_offset);
			current_pos += 512;
			current_type = (unsigned int)INodeType::INT_UNSET;
			this->fd->seekg(current_pos);
			this->fd->read(reinterpret_cast<char *>(&current_type), 2);
			continue;
		}

		if (current_type == (unsigned int)INodeType::INT_FREEBLOCK &&
			type == INodeType::INT_FILE)
		{
			// Found free block.
			this->fd->seekg(old);
			return current_pos - type_offset;
		}

		current_pos += 512;
		current_type = (unsigned int)INodeType::INT_UNSET;
		this->fd->seekg(current_pos);
		this->fd->read(reinterpret_cast<char *>(&current_type), 2);
	}

	Logging::showInfoW("The next free block is outside the current filesize.");
	Logging::showInfoO("Disk size will be increased.");
	this->fd->seekg(old);
	return current_pos - type_offset;
}

FSResult FS::addChildToDirectoryInode(unsigned int parentid, unsigned int childid)
{
	signed int type_offset = 258;
	signed int children_offset = 288;
	unsigned long pos = this->getINodePositionByID(parentid);
	std::streampos oldg = this->fd->tellg();
	std::streampos oldp = this->fd->tellp();

	// Read to make sure it's a directory.
	unsigned int parent_type = (unsigned int)INodeType::INT_UNSET;
	this->fd->seekg(pos + type_offset);
	this->fd->read(reinterpret_cast<char *>(&parent_type), 2);
	if (parent_type != INodeType::INT_DIRECTORY)
	{
		this->fd->seekg(oldg);
		this->fd->seekp(oldp);
		return FSResult::E_FAILURE_NOT_A_DIRECTORY;
	}

	// Now seek to the start of the list of directory children.
	this->fd->seekg(pos + children_offset);
	this->fd->seekp(pos + children_offset);

	// Find the first available child slot.
	unsigned int ccinode = 0;
	unsigned int count = 0;
	this->fd->read(reinterpret_cast<char *>(&ccinode), 2);
	while (ccinode != 0 && count < DIRECTORY_CHILDREN_MAX - 1)
	{
		this->fd->read(reinterpret_cast<char *>(&ccinode), 2);
		count += 1;
	}
	if (count == DIRECTORY_CHILDREN_MAX - 1 && ccinode != 0)
	{
		this->fd->seekg(oldg);
		this->fd->seekp(oldp);
		return FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED;
	}
	else
	{
		unsigned long writeg = this->fd->tellg();
		this->fd->seekp(writeg - 2);
		this->fd->write(reinterpret_cast<char *>(&childid), 2);
		this->fd->seekg(oldg);
		this->fd->seekp(oldp);
		return FSResult::E_SUCCESS;
	}
}

void FS::close()
{
	// Close the file stream.
	this->fd->close();
}