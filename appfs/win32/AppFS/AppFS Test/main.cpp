// AppFS Test.cpp : Defines the entry point for the console application.
//
#include "logging.h"
#include "fs.h"
#include "fsfile.h"
#include "util.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <iostream>

#include "config.h"
#include "global.h"
#include "tests.h"

#define RUN_TEST(func, ...) \
	std::cout << std::endl; \
	std::cout << " /================================================ " << std::endl; \
	std::cout << " |" << std::endl; \
	std::cout << " |   EXECUTING TEST '" << #func << "'" << std::endl; \
	std::cout << " |" << std::endl; \
	std::cout << " \\================================================ " << std::endl; \
	std::cout << std::endl; \
	if (func(&fs, __VA_ARGS__) != 0) \
	{ \
		AppLib::Logging::showErrorW("Test failure '%s'.  Test suite will now stop.", #func); \
		fs.close(); \
		std::cout << std::endl; \
		std::cout << " /================================================ " << std::endl; \
		std::cout << " |" << std::endl; \
		std::cout << " |  TEST SUITE STOPPED WITH ERRORS " << std::endl; \
		std::cout << " |" << std::endl; \
		std::cout << " \\================================================ " << std::endl; \
		std::cout << std::endl; \
		return 1; \
	}

int main(int argc, char* argv[])
{
	std::cout << std::endl;
	std::cout << " /================================================ " << std::endl;
	std::cout << " |" << std::endl;
	std::cout << " |  NOW TESTNG: " << std::endl;
	std::cout << " |" << std::endl;
	std::cout << " |    * AppLib 1.0 " << std::endl;
	std::cout << " |" << std::endl;
	std::cout << " |  TEST SUITE HAS STARTED " << std::endl;
	std::cout << " |" << std::endl;
	std::cout << " \\================================================ " << std::endl;
	std::cout << std::endl;

	AppLib::Logging::setApplicationName("testsuite");
	AppLib::Logging::verbose = true;
	AppLib::Logging::showInfoW("Test suite started.");

	// Create the file.
	if (!AppLib::LowLevel::Util::createPackage("test.afs", "Test App", "1.0.0", "A test package.", "AppTools"))
	{
		AppLib::Logging::showErrorW("Unable to create blank AppFS package ./test.afs.");
		return 1;
	}
	AppLib::Logging::showInfoW("Test package creation complete.");

	// Open the newly created package.
	AppLib::LowLevel::BlockStream * fd = new AppLib::LowLevel::BlockStream("test.afs");
	if (!fd->is_open())
	{
		AppLib::Logging::showErrorW("Unable to open test package.");
		return 1;
	}
	else
		AppLib::Logging::showSuccessW("Opened test package.");
	AppLib::LowLevel::FS fs(fd);

	// Set the FuseLink::filesystem for our op_ tests.
	AppLib::FUSE::FuseLink::filesystem = &fs;

	// TEST: Inspect the root inode.
	RUN_TEST(test_ll_inspect, 0);

	// TEST: Locate the next available inode number.
	uint16_t inodeid = 0;
	RUN_TEST(test_ll_next_id, inodeid);

	// TEST: Check to see if a specified filename already exists
	//       in the root directory.
	RUN_TEST(test_ll_file_unique, "file");

	// TEST: Add a new file inode.
	RUN_TEST(test_ll_allocate_node, inodeid, "file");

	// TEST: Add the new file to the root directory.
	RUN_TEST(test_ll_add_child_to_dir, inodeid, 0);

	// TEST: Fetch a list of entries in the root inode.
	RUN_TEST(test_ll_inspect, 0);

	// TEST: Set file contents.
	std::string test_data = "\
This is some test data for placement in the test file. Hopefully we can \n\
store all of this data, and it will store it correctly over multiple \n\
blocks because the filesize is larger than BSIZE_FILE - HSIZE_FILE. \n\
We should then be able to read back all of the file data later with \n\
fs.getFileContents(). \n\
\n\
This is some test data for placement in the test file. Hopefully we can \n\
store all of this data, and it will store it correctly over multiple \n\
blocks because the filesize is larger than BSIZE_FILE - HSIZE_FILE. \n\
We should then be able to read back all of the file data later with \n\
fs.getFileContents(). \n\
\n\
This is some test data for placement in the test file. Hopefully we can \n\
store all of this data, and it will store it correctly over multiple \n\
blocks because the filesize is larger than BSIZE_FILE - HSIZE_FILE. \n\
We should then be able to read back all of the file data later with \n\
fs.getFileContents(). \n\
\n\
This is some test data for placement in the test file. Hopefully we can \n\
store all of this data, and it will store it correctly over multiple \n\
blocks because the filesize is larger than BSIZE_FILE - HSIZE_FILE. \n\
We should then be able to read back all of the file data later with \n\
fs.getFileContents(). \
";
	RUN_TEST(test_rw_all_0_var, inodeid, test_data);

	// TEST: Overwrite data in an existing file (overlapping multiple segments).
	// We're going to change 512 bytes at an offset of 20 bytes within both the
	// test_data memory, and the new file we created (one byte at a time, stream)
	RUN_TEST(test_rw_stream1_20_512, inodeid, test_data);

	// TEST: Overwrite data in an existing file (overlapping multiple segments).
	// We're going to change 492 bytes at an offset of 30 bytes within both the
	// test_data memory, and the new file we created (all at once, burst)
	RUN_TEST(test_rw_burst_30_492, inodeid, test_data);

	// TEST: Locate the next available inode number.
	RUN_TEST(test_ll_next_id, inodeid);

	// TEST: Check to see if a specified filename already exists
	//       in the root directory.
	RUN_TEST(test_ll_file_unique, "file3");

	// TEST: Add a new file inode.
	RUN_TEST(test_ll_allocate_node, inodeid, "file3");

	// TEST: Add the new file to the root directory.
	RUN_TEST(test_ll_add_child_to_dir, inodeid, 0);

	// TEST: Write data into a new file.
	// We're going to write 1024 bytes at an offset of 0 bytes within both the
	// temporary memory, and the new file we created (one byte at a time, stream)
	RUN_TEST(test_rw_stream1_0_1024, inodeid);

	// TEST: Locate the next available inode number.
	RUN_TEST(test_ll_next_id, inodeid);

	// TEST: Check to see if a specified filename already exists
	//       in the root directory.
	RUN_TEST(test_ll_file_unique, "file2");

	// TEST: Add a new file inode.
	RUN_TEST(test_ll_allocate_node, inodeid, "file2");

	// TEST: Add the new file to the root directory.
	RUN_TEST(test_ll_add_child_to_dir, inodeid, 0);

	// TEST: Write data into a new file.
	// We're going to write 1000 bytes at an offset of 0 bytes within both the
	// temporary memory, and the new file we created (ten bytes at a time, stream)
	RUN_TEST(test_rw_stream10_0_1000, inodeid);

	// TEST: Locate the next available inode number.
	RUN_TEST(test_ll_next_id, inodeid);

	// TEST: Check to see if a specified filename already exists
	//       in the root directory.
	RUN_TEST(test_ll_file_unique, "file4");

	// TEST: Add a new file inode.
	RUN_TEST(test_ll_allocate_node, inodeid, "file4");

	// TEST: Add the new file to the root directory.
	RUN_TEST(test_ll_add_child_to_dir, inodeid, 0);

	// TEST: Run increase-decrease truncation test for file4.
	RUN_TEST(test_tc_incdec_400, inodeid, "/file4");

	// TEST: Run increase-decrease truncation test for file4.
	RUN_TEST(test_tc_incdec_4000, inodeid, "/file4");

	// TEST: Run increase-decrease truncation test for file4.
	RUN_TEST(test_tc_incdec_4000000, inodeid, "/file4");

	// TEST: Run random truncation test for file4.
	RUN_TEST(test_tc_rand_400, inodeid, "/file4");

	// TEST: Run Boundary-205 bug detection.
	RUN_TEST(test_bg_boundary_205, inodeid, "/file4");

	// TEST: Create a file with FuseLink::create().
	RUN_TEST(test_op_create, "file5");

	fs.close();
	std::cout << std::endl;
	std::cout << " /================================================ " << std::endl;
	std::cout << " |" << std::endl;
	std::cout << " |  TEST SUITE FINISHED SUCCESSFULLY " << std::endl;
	std::cout << " |" << std::endl;
	std::cout << " \\================================================ " << std::endl;
	std::cout << std::endl;
	return 0;
}