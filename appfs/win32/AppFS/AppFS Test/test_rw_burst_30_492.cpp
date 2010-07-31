#include "test_global.h"

int test_rw_burst_30_492(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data)
{
	char * data2 = (char*)malloc(493);
	for (int i = 0; i < 492; i += 1)
	{
		data2[i] = '2';
		test_data[i + 30] = '2';
	}
	AppLib::LowLevel::FSFile f = fs->getFile(inodeid);
	f.open();
	f.seekp(30);
	f.write(data2, 492);
	f.close();
	if (f.good())
		AppLib::Logging::showSuccessW("Burst writing for file was successful.");
	else
	{
		AppLib::Logging::showErrorW("Unable to burst write new data into the file.");
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