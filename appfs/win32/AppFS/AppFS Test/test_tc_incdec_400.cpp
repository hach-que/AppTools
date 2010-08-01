#include "test_global.h"
#include <sys/stat.h>
#include "global_tc.h"

int test_tc_incdec_400(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path)
{
	uint32_t FILESIZE = 400;

	tc_scale(FILESIZE);
	tc_setup();
	for (int a = 1; a <= 10; a += 1)
	{
		// Increase
		for (int i = 0; i <= FILESIZE; i += a)
		{
			tc_setto(i, FILESIZE, a, 10);
			AppLib::LowLevel::FSResult::FSResult res = fs->truncateFile(inodeid, i);
			if (res != 0)
			{
				printf("\n");
				AppLib::Logging::showErrorW("FuseLink truncate() operation failed for inode %i to size %i",
					inodeid, i);
				AppLib::Logging::showErrorO("while increasing in blocks of %i.", a);
				AppLib::Logging::showErrorO("  Result returned was: %s", getFSResultName(res));
				return 1;
			}
			if (!tc_verify(path, i))
			{
				printf("\n");
				AppLib::Logging::showErrorW("FuseLink truncate() operation did not resize inode %i to size %i.",
					inodeid, i);
				return 1;
			}
		}

		// Decrease
		for (int i = FILESIZE; i >= 0; i -= a)
		{
			tc_setto(i, FILESIZE, a, 10);
			AppLib::LowLevel::FSResult::FSResult res = fs->truncateFile(inodeid, i);
			if (res != 0)
			{
				printf("\n");
				AppLib::Logging::showErrorW("FuseLink truncate() operation failed for inode %i to size %i.",
					inodeid, i);
				AppLib::Logging::showErrorO("while decreasing in blocks of %i.", a);
				AppLib::Logging::showErrorO("  Result returned was: %s", getFSResultName(res));
				return 1;
			}
			if (!tc_verify(path, i))
			{
				printf("\n");
				AppLib::Logging::showErrorW("FuseLink truncate() operation did not resize inode %i to size %i.",
					inodeid, i);
				return 1;
			}
		}
	}
	
	printf("\n");
	AppLib::Logging::showSuccessW("FuseLink truncate() operation works for increase-decrease repetition to size %i",
		FILESIZE);
	return 0;
}