#include "test_global.h"

int test_ll_next_id(AppLib::LowLevel::FS * fs, uint16_t & inodeid)
{
	inodeid = fs->getFirstFreeInodeNumber();
	if (inodeid == 0)
	{
		AppLib::Logging::showErrorW("There are no free inodes available.");
		return 1;
	}
	AppLib::Logging::showInfoW("Next free inode number is: %i", inodeid);

	return 0;
}