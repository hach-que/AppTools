#include "test_global.h"
#include <sys/stat.h>
#include "global_tc.h"

int test_bg_boundary_205(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path)
{
	// Attempt to truncate the file to 200 bytes.
	AppLib::LowLevel::FSResult::FSResult res = fs->truncateFile(inodeid, 200);
	if (res != 0)
	{
		AppLib::Logging::showErrorW("Unable to truncate file to 200 bytes.");
		AppLib::Logging::showErrorO("Result of operation was %s.", getFSResultName(res));
	}

	// Now use FuseLink::write() to write 5 bytes.
	AppLib::FUSE::FuseLink::write(path, "abcde", 5, 200, NULL);

	// Check that the size is correct (should now be 205 bytes).
	if (!tc_verify(path, 205))
	{
		AppLib::Logging::showErrorW("The first write operation (off 200, write 5) failed");
		AppLib::Logging::showErrorO("to increase the file to the correct size.");
		return 1;
	}

	// Now use FuseLink::write() to write 5 bytes.
	AppLib::FUSE::FuseLink::write(path, "abcde", 5, 205, NULL);

	// Check that the size is correct (should now be 210 bytes).
	if (!tc_verify(path, 210))
	{
		AppLib::Logging::showErrorW("The second write operation (off 200, write 5) failed");
		AppLib::Logging::showErrorO("to increase the file to the correct size.  If this");
		AppLib::Logging::showErrorO("occurs, then the Boundary-205 bug exists within");
		AppLib::Logging::showErrorO("this version of AppLib.");
		return 1;
	}

	AppLib::Logging::showSuccessW("Boundary-205 bug does not occur within this version");
	AppLib::Logging::showSuccessO("of AppLib");
	return 0;
}