#include "test_global.h"

int test_file_unique(AppLib::LowLevel::FS * fs, const char* filename)
{
	if (fs->filenameIsUnique(0, const_cast<char*>(filename)) != AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		AppLib::Logging::showErrorW("The file '%s' already exists in the root directory.", filename);
		return 1;
	}
	return 0;
}