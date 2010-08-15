/*

Code file for FreeList.

This class manages the free space allocation.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                28th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "freelist.h"
#include "fs.h"
#include <math.h>

namespace AppLib
{
	namespace LowLevel
	{
		FreeList::FreeList(FS * filesystem, BlockStream * fd)
		{
			this->filesystem = filesystem;
			this->fd = fd;

			// Make a cache out of the on-disk data.
			this->syncronizeCache();
		}

		uint32_t FreeList::allocateBlock()
		{
			// Check to see if the size of the position cache is 0, in which case
			// we need to actually allocate a new block at the end of the file.
			if (this->position_cache.size() == 0)
			{
				// Get the filesize.
				std::streampos oldg = this->fd->tellg();
				this->fd->seekg(0, std::ios::end);
				uint32_t fsize = (uint32_t)this->fd->tellg();
				this->fd->seekg(oldg);

				// Align the position on the upper 4096 boundary.
				double fblocks = fsize / 4096.0f;
				uint32_t alignedpos = ceil(fblocks) * 4096;

				return alignedpos;
			}

			// Get the first unallocated block.
			std::map<uint32_t, uint32_t>::iterator i = this->position_cache.begin();

			// Update the position in the free block allocation table
			// to be equal to 0 to indicate that the free block is taken.
			std::streampos oldp = this->fd->tellp();
			this->fd->seekp(i.first);
			Endian::doW(this->fd, reinterpret_cast<char *>(&i.second), 4);
			this->fd->seekp(oldp);

			// Remove the entry from the position cache.
			this->position_cache.erase(i);

			// Return the new writable position.
			return i.second;
		}

		void FreeList::freeBlock(uint32_t pos)
		{
			// Get a new, blank writable index on the disk.
			uint32_t dpos = this->getIndexInList(0);

			// Add the new free position to the cache.
			this->position_cache.insert(std::map<uint32_t, uint32_t>::value_type(pos, dpos));
		}

		uint32_t FreeList::getIndexInList(uint32_t pos)
		{
			// Get the FSInfo inode by position.
			INode fsinfo = this->filesystem->getINodeByPosition(OFFSET_FSINFO);

			// Get the position of the first FreeList inode.
			uint32_t fpos = fsinfo.pos_freelist;
			uint32_t ipos = 0;
			uint32_t tpos = 0;

			// Store the current position of the file descriptor.
			std::streampos oldg = this->fd->tellg();

			// Loop through the FreeList inodes, searching for
			// correct index.
			while (fpos != 0)
			{
				for (int i = 8; i < 4096; i += 4)
				{
					this->fd->seekg(fpos + i);
					Endian::doR(this->fd, reinterpret_cast<char *>(&tpos), 4);
					if (tpos == pos)
					{
						// Success, we've matched correctly.
						// Seek back to the original position.
						this->fd->seekg(oldg);

						// Return the correct position.
						return fpos + i;
					}
				}
			}

			// Special condition: If the pos is 0, and ipos is 0,
			// then we need to allocate a new FreeList block for
			// storing a new value.
			if (pos == 0 && ipos == 0 && this->position_cache.size() == 0)
			{
				// Create a new FreeList block.
				uint32_t fpos = this->allocateBlock();
				if (fpos == 0)
				{
					this->fd->seekg(oldg);
					return 0;
				}
				INode fnode(0, "", INodeType::INT_FREELIST);
				FSResult::FSResult res = this->filesystem->writeINode(fpos, fnode);
				if (res != FSResult::E_SUCCESS)
				{
					this->fd->seekg(oldg);
					return 0;
				}

				// Seek back to the original position.
				this->fd->seekg(oldg);

				return fpos + HSIZE_FREELIST;
			}

			// Seek back to the original position.
			this->fd->seekg(oldg);

			return 0;
		}

		bool FreeList::isBlockFree(uint32_t pos)
		{
			return false;
		}

		INodeType::INodeType FreeList::getBlockType(uint32_t pos)
		{
			return INodeType::INT_INVALID;
		}
	}
}
