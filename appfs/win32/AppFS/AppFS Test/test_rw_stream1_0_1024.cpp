#include "test_global.h"

int test_rw_stream1_0_1024(AppLib::LowLevel::FS * fs, uint16_t inodeid)
{
	AppLib::LowLevel::FSFile f = fs->getFile(inodeid);
	f.open();
	f.seekp(0);
	char * verify_temp = (char*)malloc(1025);
	for (int i = 0; i < 1025; i += 1)
		verify_temp[i] = 0;
	const char * c = "9";
	char c2 = '9';
	for (int i = 0; i < 1024; i += 1)
	{
		switch (c2)
		{
			case '1':
				c = "2";
				c2 = '2';
				break;
			case '2':
				c = "3";
				c2 = '3';
				break;
			case '3':
				c = "4";
				c2 = '4';
				break;
			case '4':
				c = "5";
				c2 = '5';
				break;
			case '5':
				c = "6";
				c2 = '6';
				break;
			case '6':
				c = "7";
				c2 = '7';
				break;
			case '7':
				c = "8";
				c2 = '8';
				break;
			case '8':
				c = "9";
				c2 = '9';
				break;
			case '9':
				c = "0";
				c2 = '0';
				break;
			case '0':
				c = "1";
				c2 = '1';
				break;
		}
		f.write(c, 1);
		verify_temp[i] = c2;
	}
	f.close();
	if (f.good())
		AppLib::Logging::showSuccessW("Stream writing for file was successful.");
	else
	{
		AppLib::Logging::showErrorW("Unable to stream write new data into the file.");
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
			if ((*data)[i] != verify_temp[i])
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