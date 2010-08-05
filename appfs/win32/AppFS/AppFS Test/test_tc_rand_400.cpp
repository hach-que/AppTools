#include "test_global.h"
#include <sys/stat.h>
#include "global_tc.h"
#include <time.h>

int test_tc_rand_400(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path)
{
	uint32_t FILESIZE = 400;

	srand(time(NULL));

	tc_scale(FILESIZE);
	tc_setup();
	for (int a = 1; a <= 1000; a += 1)
	{
		// Randomly jump to a new size.
		int i = rand() % FILESIZE;

		tc_setto(i, FILESIZE, a, 1000);
		AppLib::LowLevel::FSResult::FSResult res = fs->truncateFile(inodeid, i);
		if (res != 0)
		{
			printf("\n");
			AppLib::Logging::showErrorW("FuseLink truncate() operation failed for");
			AppLib::Logging::showErrorO("inode %i to size %i.",
				inodeid, i);
			AppLib::Logging::showErrorO("  Result returned was: %s", getFSResultName(res));
			return 1;
		}
		if (!tc_verify(path, i))
		{
			printf("\n");
			AppLib::Logging::showErrorW("FuseLink truncate() operation did not resize");
			AppLib::Logging::showErrorO("inode %i to size %i.",
				inodeid, i);
			AppLib::Logging::showErrorO("  Expected: %i", i);
			AppLib::Logging::showErrorO("  Got     : %i", tc_getsize(path));
			return 1;
		}
	}
	
	printf("\n");
	AppLib::Logging::showSuccessW("FuseLink truncate() operation works for");
	AppLib::Logging::showSuccessO("randomization repetition to size %i",
		FILESIZE);
	return 0;
}