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
		this->fd->read(reinterpret_cast<char *>(&node.children), 4096);
	}

	// Seek back to the original reading position.
	this->fd->seekg(old);

	return node;
}

FSResult FS::writeINode(unsigned long pos, INode node)
{
	return FSResult::E_FAILURE_UNKNOWN;
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

FSResult FS::setINodePositionByID(unsigned int id, unsigned long pos)
{
	std::streampos old = this->fd->tellp();
	this->fd->seekp(OFFSET_LOOKUP + (id * 4));
	this->fd->write(reinterpret_cast<char *>(&pos), 4);
	this->fd->seekp(old);
	return FSResult::E_SUCCESS;
}

void FS::close()
{
	// Close the file stream.
	this->fd->close();
}