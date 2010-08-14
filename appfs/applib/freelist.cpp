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

namespace AppLib
{
	namespace LowLevel
	{
		FreeList::FreeList(FS * filesystem)
		{
			this->filesystem = filesystem;
		}

		uint32_t FreeList::allocateBlock()
		{
			return 0;
		}

		void FreeList::freeBlock(uint32_t pos)
		{
		}

		uint32_t FreeList::getIndexInList(uint32_t pos)
		{
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
