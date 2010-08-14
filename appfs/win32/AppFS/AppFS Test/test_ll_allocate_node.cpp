#include "test_global.h"

int test_ll_allocate_node(AppLib::LowLevel::FS * fs, uint16_t inodeid, const char* filename)
{
	uint32_t pos = fs->getFirstFreeBlock(AppLib::LowLevel::INodeType::INT_FILEINFO);
	AppLib::Logging::showInfoW("Next free data block for a file is at: %i", pos);
	time_t ltime;
	time(&ltime);
	AppLib::LowLevel::INode node(inodeid, const_cast<char*>(filename), AppLib::LowLevel::INodeType::INT_FILEINFO, 1000, 1000, 0725, ltime, ltime, ltime);
	if (fs->writeINode(pos, node) == AppLib::LowLevel::FSResult::E_SUCCESS)
		AppLib::Logging::showSuccessW("Wrote new file inode to: %i", pos);
	else
	{
		AppLib::Logging::showErrorW("Unable to write new file inode to: %i", pos);
		return 1;
	}
	return 0;
}