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
#include <errno.h>
#include <assert.h>

namespace AppLib
{
	namespace LowLevel
	{
		FS::FS(std::fstream * fd)
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
			if (ipos == 0 || ipos < OFFSET_ROOTINODE)
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
			if (node.type == INodeType::INT_SEGMENT)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_next), 4);
				
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
			if (node.type == INodeType::INT_FILE)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.dev),      2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.rdev),     2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.nlink),    2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.blocks),   2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.dat_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_next), 4);
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
			if (node.type != INodeType::INT_SEGMENT && this->getINodePositionByID(node.inodeid) != 0)
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
			if (node.type == INodeType::INT_FILE || node.type == INodeType::INT_SEGMENT ||
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
			if (node.type == INodeType::INT_FILE || node.type == INodeType::INT_SYMLINK)
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
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			std::streampos old = this->fd->tellg();

			// Since the inode number can be 0, and the filename can be blank, we
			// check the value of the type field (offset 2).  If it's 0, then
			// it's a free block.
			signed int type_offset = 2;
			uint32_t current_pos = OFFSET_DATA + type_offset;
			uint16_t current_type = (uint16_t)INodeType::INT_UNSET;
			this->fd->seekg(current_pos);
			Endian::doR(this->fd, reinterpret_cast<char *>(&current_type), 2);
			int directoryBlocksLeft = 0; // Keeps track of the number of blocks to skip
										 // regardless, because we're inside a directory
										 // inode.
			int directoryBlocksClearLeft = (BSIZE_DIRECTORY / BSIZE_FILE);
										 // Keeps track of the number of blocks that are
										 // clear when searching for a free section for a
										 // directory inode.
			while (!this->fd->eof())
			{
				if (current_type == (uint16_t)INodeType::INT_UNSET)
				{
					// Error reading.
					Logging::showErrorW("Unable to read block at position %i.", current_pos - type_offset);
					current_pos += 512;
					current_type = (uint16_t)INodeType::INT_UNSET;
					this->fd->seekg(current_pos);
					Endian::doR(this->fd, reinterpret_cast<char *>(&current_type), 2);
					continue;
				}

				if (current_type == (uint16_t)INodeType::INT_DIRECTORY)
					directoryBlocksLeft = (BSIZE_DIRECTORY / BSIZE_FILE);

				if (current_type == (uint16_t)INodeType::INT_FREEBLOCK &&
					type == INodeType::INT_FILE &&
					directoryBlocksLeft == 0)
				{
					// Found free block.
					this->fd->seekg(old);
					return current_pos - type_offset;
				}

				if (current_type == (uint16_t)INodeType::INT_FREEBLOCK &&
					type == INodeType::INT_DIRECTORY &&
					directoryBlocksLeft == 0)
				{
					if (directoryBlocksClearLeft > 0)
					{
						directoryBlocksClearLeft -= 1;
					}
					else
					{
						// Found free block.
						this->fd->seekg(old);
						return current_pos - type_offset;
					}
				}
				
				if (current_type != (uint16_t)INodeType::INT_FREEBLOCK &&
					type == INodeType::INT_DIRECTORY &&
					directoryBlocksLeft == 0)
				{
					directoryBlocksClearLeft = (BSIZE_DIRECTORY / BSIZE_FILE);
				}

				if (directoryBlocksLeft > 0)
					directoryBlocksLeft -= 1;

				current_pos += 512;
				current_type = (uint16_t)INodeType::INT_UNSET;
				this->fd->seekg(current_pos);
				Endian::doR(this->fd, reinterpret_cast<char *>(&current_type), 2);
			}

			Logging::showInfoW("The next free block is outside the current filesize.");
			Logging::showInfoO("Disk size will be increased.");
			this->fd->seekg(old);
			return current_pos - type_offset;
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
					if (cnode.type == INodeType::INT_FILE || cnode.type == INodeType::INT_DIRECTORY)
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
					if (cnode.type == INodeType::INT_FILE || cnode.type == INodeType::INT_DIRECTORY)
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
					if (cnode.type == INodeType::INT_FILE || cnode.type == INodeType::INT_DIRECTORY)
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
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Get the inode and it's position.
			uint32_t ipos = this->getINodePositionByID(id);
			INode node = this->getINodeByID(id);

			// Now we clear all of the blocks that the file
			// is currently using.
			uint32_t npos = this->getFileNextBlock(ipos);
			uint32_t opos = npos;
			while (npos != 0)
			{
				npos = this->getFileNextBlock(npos);
				this->resetBlock(opos);
				opos = npos;
			}

			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();
			if (len <= BSIZE_FILE - HSIZE_FILE)
			{
				FSResult::FSResult length_set_result = this->setFileLengthDirect(ipos, len);
				if (length_set_result != FSResult::E_SUCCESS)
					return length_set_result;

				// Our data will completely fit into the first block.
				Util::seekp_ex(this->fd, ipos + HSIZE_FILE);
				this->fd->write(data, len);
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_SUCCESS;
			}
			else
			{
				FSResult::FSResult length_set_result = this->setFileLengthDirect(ipos, len);
				if (length_set_result != FSResult::E_SUCCESS)
					return length_set_result;

				// We have to spread the file out over multiple blocks.
				// Write the first BSIZE_FILE - HSIZE_FILE bytes.
				Util::seekp_ex(this->fd, ipos + HSIZE_FILE);
				this->fd->write(data, BSIZE_FILE - HSIZE_FILE);

				// Now allocate more blocks.
				uint32_t remaining_bytes = len - (BSIZE_FILE - HSIZE_FILE);
				uint32_t data_position = BSIZE_FILE - HSIZE_FILE;
				uint32_t ppos = ipos;
				while (remaining_bytes > 0)
				{
					uint32_t pos = this->getFirstFreeBlock(INodeType::INT_FILE); // INT_FILE and INT_SEGMENT both take up a single block.
					INode nnode(node.inodeid, "", INodeType::INT_SEGMENT);
					nnode.seg_len = (remaining_bytes > (BSIZE_FILE - HSIZE_SEGMENT) ? (BSIZE_FILE - HSIZE_SEGMENT) : remaining_bytes);
					nnode.seg_next = 0;
					// Update the seg_next position of the previous inode.
					this->setFileNextSegmentDirect(ppos, pos);
					ppos = pos;
					// Write the file data.
					Util::seekp_ex(this->fd, pos);
					std::string nnode_str = nnode.getBinaryRepresentation();
					this->fd->write(nnode_str.c_str(), nnode_str.length());
					// We don't need to seek here because our current write position will be
					// where we want to write the data.
					this->fd->write(data + data_position, nnode.seg_len);
					remaining_bytes -= nnode.seg_len;
					data_position += nnode.seg_len;
				}
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);
				return FSResult::E_SUCCESS;
			}
		}

		FSResult::FSResult FS::getFileContents(uint16_t id, char ** data_out, uint32_t * len_out, uint32_t len_max)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Get the inode and it's position.
			uint32_t ipos = this->getINodePositionByID(id);
			INode node = this->getINodeByID(id);
			uint32_t len = (node.dat_len < len_max ? node.dat_len : len_max);

			// Store the length of the file or len_max in
			// len_out.
			*len_out = len;

			// Allocate a block of data with size len.
			char * data = (char*)malloc(len);
			*data_out = data;
			for (uint32_t i = 0; i < len; i += 1)
				data[i] = 0;

			std::streampos oldg = this->fd->tellg();
			if (len <= BSIZE_FILE - HSIZE_FILE)
			{
				// Our data is located entirely in the first block.
				this->fd->seekg(ipos + HSIZE_FILE);
				this->fd->read(data, len);
				this->fd->seekg(oldg);
				return FSResult::E_SUCCESS;
			}
			else
			{
				// The data will be spread out over multiple blocks.
				// Read the first BSIZE_FILE - HSIZE_FILE bytes.
				this->fd->seekg(ipos + HSIZE_FILE);
				this->fd->read(data, BSIZE_FILE - HSIZE_FILE);

				// Now read more blocks.
				uint32_t remaining_bytes = len - (BSIZE_FILE - HSIZE_FILE);
				uint32_t data_position = BSIZE_FILE - HSIZE_FILE;
				uint32_t npos = node.seg_next;
				while (remaining_bytes > 0 && npos != 0)
				{
					INode nnode = this->getINodeByPosition(npos);
					// We know that this is a INT_SEGMENT because it can not
					// be the first block in the data file (that's already
					// been read).  Therefore, seek to the INode's position +
					// HSIZE_SEGMENT.
					this->fd->seekg(npos + HSIZE_SEGMENT);
					// Now read the data.
					uint32_t read_size = nnode.seg_len;
					this->fd->read(data + data_position, read_size);
					remaining_bytes -= nnode.seg_len;
					data_position += nnode.seg_len;
					npos = nnode.seg_next;
				}
				this->fd->seekg(oldg);
				return FSResult::E_SUCCESS;
			}
		}

		FSResult::FSResult FS::setFileLengthDirect(uint32_t pos, uint32_t len)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			signed int file_len_offset = 298;
			signed int seg_len_offset = 4;

			this->fd->clear();

			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Get the type directly.
			uint16_t type_raw = (uint16_t)INodeType::INT_INVALID;
			this->fd->seekg(pos + 2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&type_raw), 2);

			if (type_raw == INodeType::INT_FILE)
			{
				Util::seekp_ex(this->fd, pos + file_len_offset);
				if (len > BSIZE_FILE - HSIZE_FILE)
				{
					Endian::doW(this->fd, reinterpret_cast<char *>(&len),  4);
					uint32_t seg_len = BSIZE_FILE - HSIZE_FILE;
					Endian::doW(this->fd, reinterpret_cast<char *>(&seg_len),  4);
				}
				else
				{
					Endian::doW(this->fd, reinterpret_cast<char *>(&len),  4);
					Endian::doW(this->fd, reinterpret_cast<char *>(&len),  4);
				}
				Util::seekp_ex(this->fd, oldp);
				this->fd->seekg(oldg);
				return FSResult::E_SUCCESS;
			}
			else if (type_raw == INodeType::INT_SEGMENT)
			{
				Util::seekp_ex(this->fd, pos + seg_len_offset);
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
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			signed int file_next_offset = 306;
			signed int seg_next_offset = 8;

			this->fd->clear();

			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Get the type directly.
			uint16_t type_raw = (uint16_t)INodeType::INT_INVALID;
			this->fd->seekg(pos + 2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&type_raw), 2);

			if (type_raw == INodeType::INT_FILE)
			{
				Util::seekp_ex(this->fd, pos + file_next_offset);
				Endian::doW(this->fd, reinterpret_cast<char *>(&seg_next),  4);
				Util::seekp_ex(this->fd, oldp);
				this->fd->seekg(oldg);
				return FSResult::E_SUCCESS;
			}
			else if (type_raw == INodeType::INT_SEGMENT)
			{
				Util::seekp_ex(this->fd, pos + seg_next_offset);
				Endian::doW(this->fd, reinterpret_cast<char *>(&seg_next),  4);
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

		uint32_t FS::getFileNextBlock(uint32_t pos)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			if (pos == 0 || pos < OFFSET_ROOTINODE)
				return 0;
			INode node(0, "", INodeType::INT_INVALID);

			// Seek to the inode position
			std::streampos old = this->fd->tellg();
			this->fd->seekg(pos);

			// Read the data.
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.inodeid),  2);
			Endian::doR(this->fd, reinterpret_cast<char *>(&node.type),     2);

			if (node.type == INodeType::INT_FILE)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.filename), 256);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.uid),      2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.gid),      2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.mask),     2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.atime),    8);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.mtime),    8);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.ctime),    8);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.dev),      2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.rdev),     2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.nlink),    2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.blocks),   2);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.dat_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_next), 4);
			}
			else if (node.type == INodeType::INT_SEGMENT)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_len),  4);
				Endian::doR(this->fd, reinterpret_cast<char *>(&node.seg_next), 4);
			}

			// Seek back to the original reading position.
			this->fd->seekg(old);

			return node.seg_next;
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
			return FSResult::E_SUCCESS;
		}

		uint32_t FS::resolvePositionInFile(uint16_t inodeid, uint32_t pos)
		{
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			uint32_t dpos = 0;
			uint32_t npos = this->getINodePositionByID(inodeid);
			uint32_t ppos = npos;
			INode node = this->getINodeByPosition(npos);
			uint32_t ptpos = 0;
			uint32_t tpos = 0;
			while (tpos <= pos)// && pos < tpos + node.seg_len)
			{
				ptpos = tpos;
				tpos += node.seg_len;
				if (tpos <= pos)
				{
					ppos = npos;
					npos = this->getFileNextBlock(npos);
					if (npos == 0)
						return 0; // Invalid position.
					node = this->getINodeByPosition(npos);
				}
				else
					break;
			}
			uint32_t noff = 0;
			if (node.type == INodeType::INT_FILE)
				noff = HSIZE_FILE;
			else if (node.type == INodeType::INT_SEGMENT)
				noff = HSIZE_SEGMENT;
			else
				return 0;

			// TODO: Update this description :P
			// Now npos is the segment in which the specified file position
			// is located.  pos - ptpos indicates the number of bytes inside
			// the segment the specified file position is.  noff indicates
			// the size of the headers for the specified block.  Hence
			// (npos + noff + (pos - ptpos) - 1) is the location, on disk, of the
			// specified file position.
			return npos + noff + (pos - ptpos);
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
			assert(/* Check the stream is not in text-mode. */ this->isValid());

			// Retrieve the INode.
			INode node = this->getINodeByID(inodeid);
			uint32_t npos = this->getINodePositionByID(inodeid);
			if (node.type != INodeType::INT_FILE)
				return FSResult::E_FAILURE_NOT_A_FILE;

			// Determine whether we're extending the file or erasing data.
			if (len == node.dat_len)
				return FSResult::E_SUCCESS; // Nothing to do.
			else if (len < node.dat_len)
			{
				// Erasing data.  Determine the amount to erase.
				uint32_t amount_to_erase = (int64_t)node.dat_len - (int64_t)len;

				// We know how many solid INT_SEGMENTs we are going
				// to have to erase by the floor'd division of
				// amount_to_erase by (BSIZE_FILE - HSIZE_SEGMENT).
				int32_t blocks_to_erase = div(amount_to_erase, BSIZE_FILE - HSIZE_SEGMENT).quot;

				// Use a std::vector to store a list of all of the positions
				// of the segments in the file, as there is no easy way to
				// traverse the segments in reverse.
				std::vector<uint32_t> segpos;
				uint32_t spos = npos;
				while (spos != 0)
				{
					segpos.insert(segpos.begin(), spos);
					spos = this->getFileNextBlock(spos);
				}

				// Now traverse the list in reverse, erasing up to blocks_to_erase
				// blocks.  Do not erase the first position in the list as that
				// is the INT_FILE inode.
				uint32_t data_blocks_erased = 0;
				for (int i = segpos.size() - 1; i > 0; i -= 1)
				{
					if (blocks_to_erase <= 0)
						break;
					this->resetBlock(segpos[i]);
					data_blocks_erased += BSIZE_FILE - HSIZE_SEGMENT;
					blocks_to_erase -= 1;
				}

				// Now set the new length of the file inode.
				node.dat_len = (int64_t)node.dat_len - (int64_t)data_blocks_erased;
				this->setFileLengthDirect(npos, node.dat_len);

				// Check to see whether or not we still have more data to erase,
				// which is highly likely as the truncation length is unlikely
				// to reside on a block boundary.
				if (len < node.dat_len)
				{
					uint32_t chars_to_erase = node.dat_len - len;
					uint32_t rpos = this->resolvePositionInFile(inodeid, len);
					if (rpos == 0)
					{
						return FSResult::E_FAILURE_PARTIAL_TRUNCATION;
					}
					// TODO: Any additional checks we should be making here before erasing?
					std::streampos posp = this->fd->tellp();
					Util::seekp_ex(this->fd, rpos);
					for (int i = 0; i < chars_to_erase; i += 1)
					{
						char zero = '\0';
						this->fd->write(&zero, 1);
					}
					Util::seekp_ex(this->fd, rpos);
					
					// Set the new file length.
					this->setFileLengthDirect(npos, (int64_t)node.dat_len - (int64_t)chars_to_erase);

					// Return success.
					return FSResult::E_SUCCESS;
				}
				else
					return FSResult::E_SUCCESS;
			}
			else if (len > node.dat_len && len > BSIZE_FILE - HSIZE_FILE)
			{
				// Adding null characters.  Determine amount to add.
				uint32_t amount_to_add = (int64_t)len - (int64_t)node.dat_len;

				// Create a variable to hold the FSResult's from operations.
				FSResult::FSResult fres;

				// Locate the last segment in the file (stored in spos).
				uint32_t spos = npos;
				uint32_t sppos = spos;
				uint32_t bytes_counted = 0;
				INode snode(0, "", INodeType::INT_INVALID);
				while (spos != 0)
				{
					snode = this->getINodeByPosition(spos);
					bytes_counted += snode.seg_len;
					sppos = spos;
					spos = this->getFileNextBlock(spos);
				}
				if (snode.type != INodeType::INT_INVALID)
					bytes_counted -= snode.seg_len;
				spos = sppos;

				uint32_t snode_datsize = 0;
				if (snode.type == INodeType::INT_FILE)
					snode_datsize = BSIZE_FILE - HSIZE_FILE;
				else if (snode.type == INodeType::INT_SEGMENT)
					snode_datsize = BSIZE_FILE - HSIZE_SEGMENT;
				else
					return FSResult::E_FAILURE_INODE_NOT_VALID;

				// Determine the current size of the last segment; we'll need to increase
				// it's length.
				if (snode.seg_len < snode_datsize)
				{
					// Check to see if we can finish the operation just by increasing the size
					// of the last segment.
					if (len - (int64_t)bytes_counted < snode_datsize)
					{
						// Now set the new length of the segment inode.
						fres = this->setFileLengthDirect(spos, len - (int64_t)bytes_counted);
						if (fres != FSResult::E_SUCCESS)
							return fres;

						// Now set the new length of the file inode.
						node.dat_len = len;
						fres = this->setFileLengthDirect(npos, node.dat_len);
						return fres;					
					}
					else
					{
						// It's outside, increase the current last segment to the maximum
						// size and then continue adding new blocks.
						fres = this->setFileLengthDirect(spos, snode_datsize);
						if (fres != FSResult::E_SUCCESS)
							return fres;
					}
				}

				// We know how many solid INT_SEGMENTs we are going
				// to have to add by the ceil'd division of
				// amount_to_add by (BSIZE_FILE - HSIZE_SEGMENT).
				int32_t blocks_to_add = div(amount_to_add, BSIZE_FILE - HSIZE_SEGMENT).quot + 1;

				// Add blocks_to_add new segments to the file.
				uint32_t nepos = 0;
				uint32_t amount_to_add_remaining = amount_to_add;
				for (int i = 0; i < blocks_to_add; i++)
				{
					uint32_t segment_size = BSIZE_FILE - HSIZE_SEGMENT;
					if (amount_to_add_remaining < BSIZE_FILE - HSIZE_SEGMENT)
						segment_size = amount_to_add_remaining;
					amount_to_add_remaining -= segment_size;

					nepos = this->getFirstFreeBlock(INodeType::INT_FILE);
					INode nenode(inodeid, "", INodeType::INT_SEGMENT);
					nenode.seg_len = segment_size;
					fres = this->writeINode(nepos, nenode);
					if (fres != FSResult::E_SUCCESS)
						return fres;
					fres = this->setFileNextSegmentDirect(spos, nepos);
					if (fres != FSResult::E_SUCCESS)
						return fres;
					spos = nepos;
				}

				// Now set the new length of the file inode.
				node.dat_len = len;
				fres = this->setFileLengthDirect(npos, node.dat_len);
				if (fres != FSResult::E_SUCCESS)
					return fres;

				return FSResult::E_SUCCESS;
			}
			else if (len > node.dat_len && len <= BSIZE_FILE - HSIZE_FILE)
			{
				// Just set the new length of the file inode.
                node.dat_len = len;
                FSResult::FSResult fres = this->setFileLengthDirect(npos, node.dat_len);
                return fres;
			}
			else
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
			while (!this->fd->eof() && cpos == block_position)
			{
				Endian::doR(this->fd, reinterpret_cast<char *>(&type_stor),  2);
				switch (type_stor)
				{
					case INodeType::INT_DIRECTORY:
						block_position += BSIZE_DIRECTORY;
						this->fd->seekg(block_position);
						break;
					case INodeType::INT_FILE:
					case INodeType::INT_SEGMENT:
					case INodeType::INT_FREEBLOCK:
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
			uint32_t newpos = this->getFirstFreeBlock(INodeType::INT_FILE);
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
