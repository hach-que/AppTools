#include "test_global.h"

int test_op_create(AppLib::LowLevel::FS * fs, const char* filename)
{
	std::string fjoin = "/";
	fjoin += filename;
	if (AppLib::FUSE::FuseLink::create(fjoin.c_str(), 0311, NULL) != 0)
	{
		AppLib::Logging::showErrorW("FuseLink create() operation failed for file '%s'.", filename);
		return 1;
	}

	// Verify that the file *actually* exists.
	if (fs->filenameIsUnique(0, const_cast<char*>(filename)) != AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		AppLib::Logging::showErrorW("FuseLink create() operation did not create file '%s' as expected.", filename);
		return 1;
	}

	AppLib::Logging::showSuccessW("FuseLink create() operation created file '%s' as expected.", filename);
	return 0;
}
