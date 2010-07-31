#include "test_global.h"

int test_rw_all_0_var(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data)
{
	AppLib::Logging::showInfoW("Test data to be written is %i bytes long.", test_data.length());
	AppLib::LowLevel::FSResult::FSResult res = fs->setFileContents(inodeid, test_data.c_str(), test_data.length());
	if (res == AppLib::LowLevel::FSResult::E_SUCCESS)
		AppLib::Logging::showSuccessW("Wrote file contents for inode %i.", inodeid);
	else
	{
		AppLib::Logging::showErrorW("Unable to write file contents for inode %i.", inodeid);
		return 1;
	}

	// Verify result.
	char ** data = (char**)malloc(sizeof(char*));
	uint32_t * len = (uint32_t*)malloc(sizeof(uint32_t));
	AppLib::LowLevel::FSResult::FSResult res2 = fs->getFileContents(inodeid, data, len, 2048);
	if (res2 == AppLib::LowLevel::FSResult::E_SUCCESS && data != NULL && len != NULL)
	{
		AppLib::Logging::showSuccessW("Read file contents for inode %i:", inodeid);
		bool matches = true;
		for (uint32_t i = 0; i < *len; i += 1)
		{
			if ((*data)[i] != test_data[i])
			{
				matches = false;
				break;
			}
		}
		if (matches)
			AppLib::Logging::showSuccessW("File contents matches what was written.");
		else
		{
			AppLib::Logging::showErrorW("File contents does not match what was written.");
			return 1;
		}
	}
	else
	{
		AppLib::Logging::showErrorW("Unable to read file contents for inode %i.", inodeid);
		return 1;
	}

	return 0;
}