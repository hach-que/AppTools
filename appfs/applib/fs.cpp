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
#include "endian.h"
#include "util.h"
#include "blockstream.h"
#include <errno.h>
#include <assert.h>

namespace AppLib
{
	namespace LowLevel
	{
		FS::FS(LowLevel::BlockStream * fd)
		{
			if (fd == NULL)
				Logging::showInternalW("NULL file descriptor passed to FS constructor.");

			// Detect the endianness here (since we don't want the user to have to
			// remember to call it - stuff would break badly if they were required to
			// and forgot to call it).
			Endian::detectEndianness();

			this->fd = fd;

#ifdef WIN32
			// Check for text-mode stream, which will break binary packages.
			uint32_t tpos = this->getTemporaryBlock();
			if (tpos != 0)
			{
				char msg[5];
				char omsg[5];
				msg[0] = 'm';
				msg[1] = '\n';
				msg[2] = 0;
				msg[3] = 0;
				msg[4] = 0;
				for (int i = 0; i < 5; i++)
					omsg[i] = 0;
				Util::seekp_ex(this->fd, tpos);
				this->fd->write(&msg[0], 3);
				this->fd->seekg(tpos + 2);
				this->fd->read(&omsg[2], 3);
				omsg[0] = 'm';
				omsg[1] = '\n';
				if (strcmp(&omsg[0], &msg[0]) != 0)
				{
					Logging::showErrorW("Text-mode stream passed to FS constructor.  Make sure");
					Logging::showErrorO("the stream is in binary mode.");
					this->fd = NULL;
					return;
				}
			}
#endif
		}

		bool FS::isValid()
		{
			return (this->fd != NULL);
		}

		INode FS::getINodeByID(uint16_t id)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Retrieve the position using our getINodePositionByID
			// function.
			uint32_t ipos = this->getINodePositionByID(id);
			if (ipos == 0 || ipos < OFFSET_FSINFO)
				return INode(0, "", INodeType::INT_INVALID);
			return this->getINodeByPosition(ipos);
		}

		INode FS::getINodeByPosition(uint32_t ipos)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			INode node(0, "", INodeType::INT_INVALID);

			// Seek to the inode position
			std::streampos old = this->fd->tellg();
			this->fd->seekg(ipos);

			// Read the data.
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.inodeid),  2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.type),     2);
			if (node.type == INodeType::INT_SEGINFO)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.info_next),  4);
				
				// Seek back to the original reading position.
				this->fd->seekg(old);

				return node;
			}
			else if (node.type == INodeType::INT_FREELIST)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.flst_next),  4);
				
				// Seek back to the original reading position.
				this->fd->seekg(old);

				return node;
			}
			else if (node.type == INodeType::INT_FSINFO)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.fs_name),		10);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.ver_major),	2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.ver_minor),	2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.ver_revision),	2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.app_name),		256);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.app_ver),		32);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.app_desc),		1024);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.app_author),	256);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.pos_root),		4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.pos_freelist),	4);
				
				// Seek back to the original reading position.
				this->fd->seekg(old);

				return node;
			}
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.filename), 256);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.uid),      2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.gid),      2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.mask),     2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.atime),    8);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.mtime),    8);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.ctime),    8);
			if (node.type == INodeType::INT_FILEINFO)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.dev),      2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.rdev),     2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.nlink),    2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.blocks),   2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.dat_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.info_next), 4);
			}
			else if (node.type == INodeType::INT_DIRECTORY)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.parent),   2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.children_count), 2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.children), DIRECTORY_CHILDREN_MAX * 2);
			}

			// Seek back to the original reading position.
			this->fd->seekg(old);

			return node;
		}

		FSResult::FSResult FS::writeINode(uint32_t pos, INode node)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Check to make sure the position is valid.
			if (pos < OFFSET_DATA)
				return FSResult::E_FAILURE_INVALID_POSITION;

			// Check to make sure the inode ID is not already assigned.
			// TODO: This needs to be updated with a full list of inode types whose inode ID should
			//       be ignored.
			if (node.type != INodeType::INT_SEGINFO && this->getINodePositionByID(node.inodeid) != 0)
				return FSResult::E_FAILURE_INODE_ALREADY_ASSIGNED;

			std::streampos old = this->fd->tellp();
			std::string data = node.getBinaryRepresentation();
			Util::seekp_ex(this->fd, pos);
			this->fd->write(data.c_str(), data.length());
			if (this->fd->fail())
				Logging::showErrorW("Write failure on write of new INode.");
			if (this->fd->bad())
				Logging::showErrorW("Write bad on write of new INode.");
			if (this->fd->eof())
				Logging::showErrorW("Write EOF on write of new INode.");
			const char* z = ""; // a const char* always has a \0 terminator, which we use to write into the file.
			// TODO: This needs to be updated with a full list of inode types.
			if (node.type == INodeType::INT_FILEINFO || node.type == INodeType::INT_SEGINFO ||
				node.type == INodeType::INT_SYMLINK)
			{
				for (int i = 0; i < BSIZE_FILE - data.length(); i += 1)
					Endian::doW(this->fd, z, 1);
			}
			else if (node.type == INodeType::INT_DIRECTORY)
			{
				int target = BSIZE_DIRECTORY - data.length();
				for (int i = 0; i < target; i += 1)
					Endian::doW(this->fd, z, 1);
			}
			else
				return FSResult::E_FAILURE_INODE_NOT_VALID;
			if (node.type == INodeType::INT_FILEINFO || node.type == INodeType::INT_SYMLINK)
				this->setINodePositionByID(node.inodeid, pos);
			Util::seekp_ex(this->fd, old);
			return FSResult::E_SUCCESS;
		}

		FSResult::FSResult FS::updateINode(INode node)
        {
            assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Check to make sure the inode ID is not already assigned.
			uint32_t pos = this->getINodePositionByID(node.inodeid);
            if (pos == 0)
                return FSResult::E_FAILURE_INODE_NOT_ASSIGNED;

            std::streampos old = this->fd->tellp();
            std::string data = node.getBinaryRepresentation();
            Util::seekp_ex(this->fd, pos);
            this->fd->write(data.c_str(), data.length());
			// We do not write out the file data with zeros
			// as in writeINode because we want to keep the
			// content.
            this->setINodePositionByID(node.inodeid, pos);
            Util::seekp_ex(this->fd, old);
            return FSResult::E_SUCCESS;
        }

		uint32_t FS::getINodePositionByID(uint16_t id)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			this->fd->clear();
			std::streampos old = this->fd->tellg();
			uint32_t newp = OFFSET_LOOKUP + (id * 4);
			this->fd->seekg(newp);
			uint32_t ipos = 0;
			Endian::doR(this->fd, reinterpret_cast<char *>(&ipos), 4);
			this->fd->seekg(old);
			return ipos;
		}

		uint16_t FS::getFirstFreeInodeNumber()
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			this->fd->clear();
			std::streampos old = this->fd->tellg();
			this->fd->seekg(OFFSET_LOOKUP);
			uint32_t ipos = 0;
			uint16_t count = 0;
			uint16_t ret = 0;
			Endian::doR(this->fd, reinterpret_cast<char *>(&ipos), 4);
			while (ipos != 0 && count < 65535)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&ipos), 4);
				count += 1;
			}
			if (count == 65535 && ipos != 0)
				ret = 0;
			else
				ret = count;
			this->fd->seekg(old);
			return ret;
		}

		FSResult::FSResult FS::setINodePositionByID(uint16_t id, uint32_t pos)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			std::streampos old = this->fd->tellp();
			Util::seekp_ex(this->fd, OFFSET_LOOKUP + (id * 4));
			Endian::doW(this->fd, reinterpret_cast<char *>(&pos), 4);
			Util::seekp_ex(this->fd, old);
			return FSResult::E_SUCCESS;
		}

		uint32_t FS::getFirstFreeBlock(INodeType::INodeType type)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		FSResult::FSResult FS::addChildToDirectoryInode(uint16_t parentid, uint16_t childid)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			signed int type_offset = 2;
			signed int children_count_offset = 292;
			signed int children_offset = 294;
			uint32_t pos = this->getINodePositionByID(parentid);
			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Read to make sure it's a directory.
			uint16_t parent_type = (uint16_t)INodeType::INT_UNSET;
			this->fd->seekg(pos + type_offset);
			Endian::doR(this->fd, reinterpret_cast<char *>(&parent_type), 2);
			if (parent_type != INodeType::INT_DIRECTORY)
			{
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_FAILURE_NOT_A_DIRECTORY;
			}

			// Now seek to the start of the list of directory children.
			this->fd->seekg(pos + children_offset);
			Util::seekp_ex(this->fd, pos + children_offset);

			// Find the first available child slot.
			uint16_t ccinode = 0;
			uint16_t count = 0;
			Endian::doR(this->fd, reinterpret_cast<char *>(&ccinode), 2);
			while (ccinode != 0 && count < DIRECTORY_CHILDREN_MAX - 1)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&ccinode), 2);
				count += 1;
			}
			if (count == DIRECTORY_CHILDREN_MAX - 1 && ccinode != 0)
			{
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED;
			}
			else
			{
				uint32_t writeg = this->fd->tellg();
				Util::seekp_ex(this->fd, writeg - 2);
				Endian::doW(this->fd, reinterpret_cast<char *>(&childid), 2);

				uint16_t children_count_current = 0;
				this->fd->seekg(pos + children_count_offset);
				Endian::doR(this->fd, reinterpret_cast<char *>(&children_count_current), 2);
				children_count_current += 1;
				Util::seekp_ex(this->fd, pos + children_count_offset);
				Endian::doW(this->fd, reinterpret_cast<char *>(&children_count_current), 2);
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_SUCCESS;
			}
		}

		FSResult::FSResult FS::removeChildFromDirectoryInode(uint16_t parentid, uint16_t childid)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			signed int type_offset = 2;
			signed int children_count_offset = 292;
			signed int children_offset = 294;
			uint32_t pos = this->getINodePositionByID(parentid);
			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Read to make sure it's a directory.
			uint16_t parent_type = (uint16_t)INodeType::INT_UNSET;
			this->fd->seekg(pos + type_offset);
			Endian::doR(this->fd, reinterpret_cast<char *>(&parent_type), 2);
			if (parent_type != INodeType::INT_DIRECTORY)
			{
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_FAILURE_NOT_A_DIRECTORY;
			}

			// Now seek to the start of the list of directory children.
			this->fd->seekg(pos + children_offset);
			Util::seekp_ex(this->fd, pos + children_offset);

			// Find the slot that the child inode is in.
			uint16_t ccinode = 0;
			uint16_t count = 0;
			Endian::doR(this->fd, reinterpret_cast<char *>(&ccinode), 2);
			while (ccinode != childid && count < DIRECTORY_CHILDREN_MAX - 1)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&ccinode), 2);
				count += 1;
			}
			if (count == DIRECTORY_CHILDREN_MAX - 1 && ccinode != childid)
			{
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_FAILURE_INVALID_FILENAME;
			}
			else
			{
				uint32_t writeg = this->fd->tellg();
				Util::seekp_ex(this->fd, writeg - 2);
				uint16_t zeroid = 0;
				Endian::doW(this->fd, reinterpret_cast<char *>(&zeroid), 2);

				uint16_t children_count_current = 0;
				this->fd->seekg(pos + children_count_offset);
				Endian::doR(this->fd, reinterpret_cast<char *>(&children_count_current), 2);
				children_count_current -= 1;
				Util::seekp_ex(this->fd, pos + children_count_offset);
				Endian::doW(this->fd, reinterpret_cast<char *>(&children_count_current), 2);
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_SUCCESS;
			}
		}

		FSResult::FSResult FS::filenameIsUnique(uint16_t parentid, char * filename)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			std::vector<INode> inodechildren = this->getChildrenOfDirectory(parentid);
			for (unsigned int i = 0; i < inodechildren.size(); i += 1)
			{
				if (strcmp(filename, inodechildren[i].filename) == 0)
				{
					return FSResult::E_FAILURE_NOT_UNIQUE;
				}
			}
			return FSResult::E_SUCCESS; // Indicates unique.
		}

		std::vector<INode> FS::getChildrenOfDirectory(uint16_t parentid)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			std::vector<INode> inodechildren;
			INode node = this->getINodeByID(parentid);
			if (node.type == INodeType::INT_INVALID)
			{
				return inodechildren;
			}

			uint16_t children_looped = 0;
			uint16_t total_looped = 0;
			while (children_looped < node.children_count && total_looped < DIRECTORY_CHILDREN_MAX)
			{
				uint16_t cinode = node.children[total_looped];
				total_looped += 1;
				if (cinode == 0)
				{
					continue;
				}
				else
				{
					children_looped += 1;
					INode cnode = this->getINodeByID(cinode);
					if (cnode.type == INodeType::INT_FILEINFO || cnode.type == INodeType::INT_DIRECTORY)
					{
						inodechildren.insert(inodechildren.end(), cnode);
					}
				}
			}

			return inodechildren;
		}

		INode FS::getChildOfDirectory(uint16_t parentid, uint16_t childid)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			INode node = this->getINodeByID(parentid);
			if (node.type == INodeType::INT_INVALID)
			{
				return INode(0, "", INodeType::INT_INVALID);
			}

			uint16_t children_looped = 0;
			uint16_t total_looped = 0;
			while (children_looped < node.children_count && total_looped < DIRECTORY_CHILDREN_MAX)
			{
				uint16_t cinode = node.children[total_looped];
				total_looped += 1;
				if (cinode == 0)
				{
					continue;
				}
				else
				{
					children_looped += 1;
					INode cnode = this->getINodeByID(cinode);
					if (cnode.type == INodeType::INT_FILEINFO || cnode.type == INodeType::INT_DIRECTORY)
					{
						if (cnode.inodeid == childid)
						{
							return cnode;
						}
					}
				}
			}

			return INode(0, "", INodeType::INT_INVALID);
		}

		INode FS::getChildOfDirectory(uint16_t parentid, const char * filename)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			INode node = this->getINodeByID(parentid);
			if (node.type == INodeType::INT_INVALID)
			{
				return INode(0, "", INodeType::INT_INVALID);
			}

			uint16_t children_looped = 0;
			uint16_t total_looped = 0;
			while (children_looped < node.children_count && total_looped < DIRECTORY_CHILDREN_MAX)
			{
				uint16_t cinode = node.children[total_looped];
				total_looped += 1;
				if (cinode == 0)
				{
					continue;
				}
				else
				{
					children_looped += 1;
					INode cnode = this->getINodeByID(cinode);
					if (cnode.type == INodeType::INT_FILEINFO || cnode.type == INodeType::INT_DIRECTORY)
					{
						if (strcmp(filename, cnode.filename) == 0)
						{
							return cnode;
						}
					}
				}
			}

			return INode(0, "", INodeType::INT_INVALID);
		}

		FSResult::FSResult FS::setFileContents(uint16_t id, const char * data, uint32_t len)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		FSResult::FSResult FS::getFileContents(uint16_t id, char ** data_out, uint32_t * len_out, uint32_t len_max)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		FSResult::FSResult FS::setFileLengthDirect(uint32_t pos, uint32_t len)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			signed int file_len_offset = 298;

			this->fd->clear();

			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Get the type directly.
			uint16_t type_raw = (uint16_t)INodeType::INT_INVALID;
			this->fd->seekg(pos + 2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&type_raw), 2);

			if (type_raw == INodeType::INT_FILEINFO)
			{
				Util::seekp_ex(this->fd, pos + file_len_offset);
				Endian::doW(this->fd, reinterpret_cast<char *>(&len),  4);
				Util::seekp_ex(this->fd, oldp);
				this->fd->seekg(oldg);
				return FSResult::E_SUCCESS;
			}
			else
			{
				this->fd->seekg(oldg);
				return FSResult::E_FAILURE_INVALID_POSITION;
			}
		}

		FSResult::FSResult FS::setFileNextSegmentDirect(uint32_t pos, uint32_t seg_next)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		uint32_t FS::getFileNextBlock(uint32_t pos)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		FSResult::FSResult FS::resetBlock(uint32_t pos)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			std::streampos oldp = this->fd->tellp();
			Util::seekp_ex(this->fd, pos);
			const char * zero = "\0";
			for (int i = 0; i < BSIZE_FILE; i += 1)
			{
				this->fd->write(zero, 1);
			}
			Util::seekp_ex(this->fd, oldp);

			// TODO: Block must be marked as unused through the
			//       free list allocation class.

			return FSResult::E_SUCCESS;
		}

		uint32_t FS::resolvePositionInFile(uint16_t inodeid, uint32_t pos)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		int32_t FS::resolvePathnameToInodeID(const char * path)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			std::vector<std::string> components;
			std::string buf = "";
			for (int i = 0; i < strlen(path); i += 1)
			{
				if (path[i] == '/')
				{
					if (buf != "")
					{
						components.insert(components.end(), buf);
						buf = "";
					}
				}
				else
				{
					buf += path[i];
				}
			}
			if (buf.length() > 0)
			{
				components.insert(components.end(), buf);
				buf = "";
			}

			uint16_t id = 0;
			for (int i = 0; i < components.size(); i += 1)
			{
				if (components[i] == ".")
					id = id;
				else if (components[i] == "..")
				{
					INode node = this->getINodeByID(id);
					if (node.type == INodeType::INT_INVALID)
						return -ENOENT;
					id = node.parent;
				}
				else
				{
					INode node = this->getChildOfDirectory(id, components[i].c_str());
					if (node.type == INodeType::INT_INVALID)
						return -ENOENT;
					id = node.inodeid;
				}
			}
			return id;
		}

		FSResult::FSResult FS::truncateFile(uint16_t inodeid, uint32_t len)
		{
			// TODO: Must be reimplemented in New File Storage system.
			return FSResult::E_FAILURE_NOT_IMPLEMENTED;
		}

		FSFile FS::getFile(uint16_t inodeid)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			return FSFile(this, this->fd, inodeid);
		}

		uint32_t FS::getTemporaryBlock(bool forceRecheck)
		{
			// We use a static variable to speed up later calls.
			static uint32_t temporary_position = 0;
			if (temporary_position != 0 && !forceRecheck)
				return temporary_position;

			uint32_t block_position = OFFSET_DATA + 2;
			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Run through each of the blocks and check to see whether
			// they are a temporary block or not.
			this->fd->seekg(block_position);
			uint16_t type_stor = INodeType::INT_UNSET;
			uint32_t cpos = this->fd->tellg();
			while (!this->fd->eof() && cpos == block_position && !this->freelist->isBlockFree(cpos))
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&type_stor),  2);
				switch (type_stor)
				{
					case INodeType::INT_DIRECTORY:
						block_position += BSIZE_DIRECTORY;
						this->fd->seekg(block_position);
						break;
					case INodeType::INT_FILEINFO:
					case INodeType::INT_SEGINFO:
						block_position += BSIZE_FILE;
						this->fd->seekg(block_position);
						break;
					case INodeType::INT_INVALID:
						this->fd->seekg(oldg);
						return 0;
					case INodeType::INT_TEMPORARY:
						this->fd->seekg(oldg);
						temporary_position = block_position + 2;
						return block_position + 2;
					case INodeType::INT_UNSET:
						break;
					default:
						this->fd->seekg(oldg);
						return 0;
				}
			}
			this->fd->seekg(oldg);

			// No temporary block allocated; allocate a new one.
			uint32_t newpos = this->getFirstFreeBlock(INodeType::INT_FILEINFO);
			if (newpos == 0) return 0;
			Util::seekp_ex(this->fd, newpos);
			uint16_t zero = 0;
			uint16_t tempid = INodeType::INT_TEMPORARY;
			Endian::doW(this->fd, reinterpret_cast<char *>(&zero),  2);
			Endian::doW(this->fd, reinterpret_cast<char *>(&tempid),  2);
			Util::seekp_ex(this->fd, oldp);
			newpos += 4;
			temporary_position = newpos;
			return newpos;
		}

		void FS::close()
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Close the file stream.
			this->fd->close();
		}

		std::vector<std::string> FS::splitPathBySeperators(std::string path)
		{
			std::vector<std::string> ret;
			std::string buf = "";
			for (unsigned int i = 0; i < path.length(); i += 1)
			{
				if (path[i] == '/' && buf.length() > 0)
				{
					ret.insert(ret.end(), buf);
					buf = "";
				}
				else if (path[i] != '/')
				{
					buf += path[i];
				}
			}
			if (buf.length() > 0)
			{
				ret.insert(ret.end(), buf);
				buf = "";
			}
			return ret;
		}
	}
}
