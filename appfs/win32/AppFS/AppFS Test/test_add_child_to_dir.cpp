#include "test_global.h"

int test_add_child_to_dir(AppLib::LowLevel::FS * fs, uint16_t child, uint16_t parent)
{
	if (fs->addChildToDirectoryInode(parent, child) == AppLib::LowLevel::FSResult::E_SUCCESS)
		AppLib::Logging::showSuccessW("Added child inode %i to parent inode %i.", child, parent);
	else
	{
		AppLib::Logging::showErrorW("Unable to add child inode %i to parent inode %i.", child, parent);
		return 1;
	}

	return 0;
}