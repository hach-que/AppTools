#include "fsfile.h"
#include "fs.h"
#include "util.h"
#include "logging.h"
#include <map>
#include <math.h>

namespace AppLib
{
	namespace LowLevel
	{
		FSFile::FSFile(FS * filesystem, std::fstream * fd, uint16_t inodeid)
		{
			this->inodeid = inodeid;
			this->filesystem = filesystem;
			this->fd = fd;
			this->opened = false;
			this->invalid = false;
			this->posg = 0;
			this->posp = 0;
			this->state = std::ios::goodbit;
		}

		void FSFile::open(std::ios_base::openmode mode)
		{
			INode node = this->filesystem->getINodeByID(this->inodeid);
			this->invalid = (node.type != INodeType::INT_FILE);
			if (this->invalid)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			// TODO: Check permissions of current user / group
			//       to see whether it can be opened.  If it fails
			//       to open, set the badbit on.
			this->opened = true;

			if (false /* <failed> */)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
			}
		}

		void FSFile::write(const char * data, std::streamsize count)
		{
			if (this->invalid || !this->opened)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			// Write the data to the currently opened inode.
			std::streampos pos = this->filesystem->resolvePositionInFile(this->inodeid, this->posp);

			if ((int)pos == 0)
			{
				this->truncate(this->posp);
				pos = this->filesystem->resolvePositionInFile(this->inodeid, this->posp);
				if ((int)pos == 0)
				{
					Logging::showErrorW("Unable to expand file for write() operation.");
					this->clear(std::ios::eofbit | std::ios::badbit | std::ios::failbit);
					return;
				}
			}

/*			if ((int)pos == 0)
			{
				// We need to allocate enough blank blocks to reach the new
				// write area.
				uint32_t ppos = this->filesystem->getINodePositionByID(this->inodeid);
				uint32_t balloc = 0;
				uint32_t tpos = this->filesystem->getFileNextBlock(ppos);
				Logging::showInternalW("INode position is %i.", ppos);
				while (tpos != 0)
				{
					Logging::showInternalO("Segment position is %i.", tpos);
					ppos = tpos;
					tpos = this->filesystem->getFileNextBlock(tpos);
				}
				while ((int)pos == 0)
				{
					uint32_t npos = this->filesystem->getFirstFreeBlock(INodeType::INT_FILE);
					Logging::showInternalW("Allocating block at %i", npos);
					INode nnode(this->inodeid, "", INodeType::INT_SEGMENT);
					nnode.seg_len = BSIZE_FILE - HSIZE_SEGMENT;
					nnode.seg_next = 0;
					Logging::showInternalW("Assigning link: %i -> links to -> %i", ppos, npos);
					FSResult::FSResult res = this->filesystem->setFileNextSegmentDirect(ppos, npos);
					Logging::showInternalW("Result of set next segment is %i", res);
					ppos = npos;
					res = this->filesystem->writeINode(npos, nnode);
					Logging::showInternalW("Result of new INode allocation is %i", res);
					balloc += 1;
					Logging::showInternalW("%i more blocks allocated during FSFile::write().", balloc);

					pos = this->filesystem->resolvePositionInFile(this->inodeid, this->posp);
				}
				FSResult::FSResult resw = this->filesystem->setFileLengthDirect(ppos, pos - ppos);
				Logging::showInternalW("Result of segment length adjustment (set to %i) is %i", (int)(pos - ppos), resw);
				resw = this->filesystem->setFileLengthDirect(
					this->filesystem->getINodePositionByID(this->inodeid),
					this->posp);
				Logging::showInternalW("Result of file length adjustment (set to %i) is %i", (int)(this->posp), resw);
			}
*/

			// Our inode start position will be (pos - OFFSET_DATA) / BSIZE_FILE, since we know the
			// position won't be pointing inside a directory inode.
			std::streampos ipos = floor((float)((float)pos - OFFSET_DATA) / (float)BSIZE_FILE) * BSIZE_FILE + OFFSET_DATA;

			// Now grab the first inode.
			INode node = this->filesystem->getINodeByPosition(ipos);

			// The start of the data will be (pos - ipos) - HSIZE_FILE or (pos - ipos) - HSIZE_SEGMENT
			// depending on the node type.
			std::streampos dstart = 0;
			if (node.type == INodeType::INT_FILE)
			{
				dstart = (pos - ipos) - HSIZE_FILE;
			}
			else if (node.type == INodeType::INT_SEGMENT)
			{
				dstart = (pos - ipos) - HSIZE_SEGMENT;
			}
			else
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			// Mark the current position in the disk image.
			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Check to see whether the data we are going to write all
			// fits inside the one block.
			if (count < ((int64_t)BSIZE_FILE - dstart))
			{
				// The data we are writing is going to fit inside the
				// first block, so just write it and return.
				Util::seekp_ex(this->fd, pos);
				this->fd->write(data, count);
				this->posp += count;
			
				// Seek back to the old position.
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);

				// Update the dat_len field of the file because we
				// may have now extended the file length.
				if (this->filesystem->getFileNextBlock(ipos) == 0)
				{
					// Calculate the new file length.
					// dstart   is the number of bytes before the writing position
					//          in the current inode.
					// .seg_len will be the total number of bytes that used to be in
					//          the inode.
					// count    is the number of bytes written.
					// Therefore, ((dstart + count) - .seg_len) is the number of bytes
					// that this segment was extended by.  Adding .dat_len will get
					// the new filesize.
					int64_t extsize = (((int64_t)dstart + (int64_t)count) - (int64_t)node.seg_len);
					if (extsize > 0)
					{
						this->filesystem->setFileLengthDirect(ipos, node.dat_len + extsize);
					}
				}

				return;
			}

			// The data we are writing is going to be written out over
			// multiple blocks.  We write the first section directly.
			Util::seekp_ex(this->fd, pos);
			this->fd->write(data, node.seg_len - dstart);
			this->posp += node.seg_len - dstart;

			// Now write the remaining data into the other blocks (allocate
			// if necessary).
			uint32_t remaining_bytes = count - (node.seg_len - dstart);
			uint32_t data_position = node.seg_len - dstart;
			uint32_t ppos = ipos;
			while (remaining_bytes > 0)
			{
				uint32_t pos = this->filesystem->getFileNextBlock(ppos);
				
				// If pos is 0, then we are writing past the end of the file.
				// Hence, allocate a new block.
				if (pos == 0)
				{
					pos = this->filesystem->getFirstFreeBlock(INodeType::INT_FILE); // INT_FILE and INT_SEGMENT both take up a single block.
					INode nnode(node.inodeid, "", INodeType::INT_SEGMENT);
					nnode.seg_len = (remaining_bytes > (BSIZE_FILE - HSIZE_SEGMENT) ? (BSIZE_FILE - HSIZE_SEGMENT) : remaining_bytes);
					nnode.seg_next = 0;
					
					// Update the seg_next position of the previous inode.
					this->filesystem->setFileNextSegmentDirect(ppos, pos);
					ppos = pos;

					// Write the file data.
					Util::seekp_ex(this->fd, pos);
					std::string nnode_str = nnode.getBinaryRepresentation();
					this->fd->write(nnode_str.c_str(), nnode_str.length());
					
					// We don't need to seek here because our current write position will be
					// where we want to write the data.
					this->fd->write(data + data_position, nnode.seg_len);

					this->posp += nnode.seg_len;
					remaining_bytes -= nnode.seg_len;
					data_position += nnode.seg_len;

					// Update the dat_len field of the file because we have
					// now extended the file length.
					this->filesystem->setFileLengthDirect(ipos, data_position);
				}
				else
				{
					// Get the inode located at pos.
					INode nnode = this->filesystem->getINodeByPosition(pos);
					ppos = pos;
					uint32_t amount_to_write = (remaining_bytes > (BSIZE_FILE - HSIZE_SEGMENT) ? (BSIZE_FILE - HSIZE_SEGMENT) : remaining_bytes);

					// Seek to the start of the data.  This will always be a
					// SEGMENT node since we're already written in the first block.
					Util::seekp_ex(this->fd, pos + HSIZE_SEGMENT);

					// Write the data to the inode.
					this->fd->write(data + data_position, amount_to_write);
					this->posp += amount_to_write;
					remaining_bytes -= amount_to_write;
					data_position += amount_to_write;
				}
			}

			// Seek back to the old position.
			this->fd->seekg(oldg);
			Util::seekp_ex(this->fd, oldp);
		}

		std::streamsize FSFile::read(char * out, std::streamsize count)
		{
			if (this->invalid || !this->opened)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

			// Read the data from the currently opened inode.
			std::streampos pos = this->filesystem->resolvePositionInFile(this->inodeid, this->posg);

			if ((int)pos == 0)
			{
				this->clear(std::ios::badbit | std::ios::eofbit);
				return 0;
			}

			// Our inode start position will be (pos - OFFSET_DATA) / BSIZE_FILE, since we know the
			// position won't be pointing inside a directory inode.
			std::streampos ipos = floor((float)((float)pos - OFFSET_DATA) / (float)BSIZE_FILE) * BSIZE_FILE + OFFSET_DATA;

			// Now grab the first inode.
			INode node = this->filesystem->getINodeByPosition(ipos);

			// The start of the data will be (pos - ipos) - HSIZE_FILE or (pos - ipos) - HSIZE_SEGMENT
			// depending on the node type.
			std::streampos dstart = 0;
			if (node.type == INodeType::INT_FILE)
			{
				dstart = (pos - ipos) - HSIZE_FILE;
			}
			else if (node.type == INodeType::INT_SEGMENT)
			{
				dstart = (pos - ipos) - HSIZE_SEGMENT;
			}
			else
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return 0;
			}

			// Mark the current position in the disk image.
			std::streampos oldg = this->fd->tellg();
			std::streampos oldp = this->fd->tellp();

			// Check to see whether the data we are going to read all
			// fits inside the one block.
			if ((count < node.dat_len ? count : node.dat_len) < ((int64_t)BSIZE_FILE - dstart))
			{
				// The data we are reading is all contained within the
				// first block.  Read it and then return.
				Util::seekp_ex(this->fd, pos);

				// Read either count blocks or node.seg_dat blocks, which
				// ever is smaller.
				if (count < node.dat_len)
				{
					this->fd->read(out, count);
					this->posg += count;
				}
				else
				{
					this->fd->read(out, node.dat_len);
					this->posg += node.dat_len;
				}
			
				// Seek back to the old position.
				this->fd->seekg(oldg);
				Util::seekp_ex(this->fd, oldp);

				if (count < node.dat_len)
					return count;
				else
					return node.dat_len;
			}

			// The data we are reading is going to be read over
			// multiple blocks.  We read the first section directly.
			Util::seekp_ex(this->fd, pos);
			this->fd->read(out, node.seg_len - dstart);
			this->posg += node.seg_len - dstart;

			// Now read the remaining data from the other blocks.
			uint32_t remaining_bytes = count - (node.seg_len - dstart);
			uint32_t data_position = node.seg_len - dstart;
			uint32_t ppos = ipos;
			while (remaining_bytes > 0)
			{
				uint32_t pos = this->filesystem->getFileNextBlock(ppos);
				
				// If pos is 0, then we can't read any more data.  Return the
				// current amount read.
				if (pos == 0)
				{
					return (int64_t)count - (int64_t)remaining_bytes;
				}
				else
				{
					// Get the inode located at pos.
					INode nnode = this->filesystem->getINodeByPosition(pos);
					ppos = pos;
					uint32_t amount_to_read = (remaining_bytes > (BSIZE_FILE - HSIZE_SEGMENT) ? (BSIZE_FILE - HSIZE_SEGMENT) : remaining_bytes);

					// Seek to the start of the data.  This will always be a
					// SEGMENT node since we're already read in the first block.
					Util::seekp_ex(this->fd, pos + HSIZE_SEGMENT);

					// Read the data from the inode.
					this->fd->read(out + data_position, amount_to_read);
					this->posg += amount_to_read;
					remaining_bytes -= amount_to_read;
					data_position += amount_to_read;
				}
			}

			// Seek back to the old position.
			this->fd->seekg(oldg);
			Util::seekp_ex(this->fd, oldp);

			return count;
		}

		void FSFile::truncate(std::streamsize len)
		{
			this->filesystem->truncateFile(this->inodeid, len);
		}

		void FSFile::close()
		{
			this->opened = false;
		}

		void FSFile::seekp(std::streampos pos)
		{
			if (this->invalid || !this->opened)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}

			this->posp = pos;
		}

		void FSFile::seekg(std::streampos pos)
		{
			if (this->invalid || !this->opened)
			{
				this->clear(std::ios::badbit | std::ios::failbit);
				return;
			}
			
			this->posg = pos;
		}

		std::streampos FSFile::tellp()
		{
			return this->posp;
		}

		std::streampos FSFile::tellg()
		{
			return this->posg;
		}

		std::ios::iostate FSFile::rdstate()
		{
			return this->state;
		}

		void FSFile::clear()
		{
			this->state = std::ios::goodbit;
		}

		void FSFile::clear(std::ios::iostate state)
		{
			this->state = state;
		}

		bool FSFile::good()
		{
			return (this->state == std::ios::goodbit);
		}

		bool FSFile::bad()
		{
			return (this->state & std::ios::badbit != 0);
		}

		bool FSFile::eof()
		{
			return (this->state & std::ios::eofbit != 0);
		}

		bool FSFile::fail()
		{
			return (this->state & std::ios::failbit != 0);
		}
	}
}
