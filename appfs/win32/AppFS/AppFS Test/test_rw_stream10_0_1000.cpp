#include "test_global.h"

int test_rw_stream10_0_1000(AppLib::LowLevel::FS * fs, uint16_t inodeid)
{
	AppLib::LowLevel::FSFile f = fs->getFile(inodeid);
	f.open();
	f.seekp(0);
	char * verify_temp = (char*)malloc(1001);
	for (int i = 0; i < 1001; i += 1)
		verify_temp[i] = 0;
	const char * c3 = "0123456789";
	for (int i = 0; i < 1000 / 10; i += 1)
	{
		f.write(c3, 10);
		for (int a = 0; a < 10; a += 1)
			verify_temp[i+a] = c3[a];
	}
	f.close();
	if (f.good())
		AppLib::Logging::showSuccessW("Stream writing (new) for file was successful.");
	else
	{
		AppLib::Logging::showErrorW("Unable to stream write new data into the file.");
		return 1;
	}

	// TEST: Confirm success of stream writing.
	char ** data = (char**)malloc(sizeof(char*));
	uint32_t * len = (uint32_t*)malloc(sizeof(uint32_t));
	AppLib::LowLevel::FSResult::FSResult res2 = fs->getFileContents(inodeid, data, len, 2048);
	if (res2 == AppLib::LowLevel::FSResult::E_SUCCESS && data != NULL && len != NULL)
	{
		std::cout << "Read file contents for inode "  << std::endl;
		bool matches = true;
		for (uint32_t i = 0; i < *len; i += 1)
		{
			if ((*data)[i] != verify_temp[i])
			{
				matches = false;
				break;
			}
		}
		if (matches)
			std::cout << "File contents matches what was written." << std::endl;
		else
		{
			std::cout << "File contents does not match what was written." << std::endl;
			return 1;
		}
	}
	else
	{
		std::cout << "Unable to read file contents for inode %i." << std::endl;
		return 1;
	}

	return 0;
}