// AppFS Test.cpp : Defines the entry point for the console application.
//
#include "logging.h"
#include "fs.h"
#include "fsfile.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <iostream>

#include "config.h"

bool file_exists(std::string filename)
{
	struct stat file_info;
	return (stat(filename.c_str(), &file_info) == 0);
}

void printListHeaders(uint16_t id, uint16_t total)
{
	Logging::showInfoW("inode %i, total %i:", id, total);
	Logging::showInfoO("MASK       NODID   UID   GID      SIZE DATETIME     FNAME");
}

void getFormattedPermissionBlock(uint8_t m, char * out)
{
	const char * a = "---";
	switch (m)
	{
		case 0: a = "---"; break;
		case 1: a = "r--"; break;
		case 2: a = "-w-"; break;
		case 3: a = "rw-"; break;
		case 4: a = "--x"; break;
		case 5: a = "r-x"; break;
		case 6: a = "-wx"; break;
		case 7: a = "rwx"; break;
	}
	strcpy(out, a);
}

void getFormattedPermissionMask(uint16_t mask, INodeType type, char * out)
{
	for (int i = 0; i < 11; i += 1)
		out[i] = '\0';
	out[0] = '-';
	switch (type)
	{
		case INodeType::INT_FREEBLOCK:	out[0] = '_'; break;
		case INodeType::INT_FILE:		out[0] = '-'; break;
		case INodeType::INT_DIRECTORY:	out[0] = 'd'; break;
		case INodeType::INT_SEGMENT:	out[0] = '#'; break;
		case INodeType::INT_INVALID:	out[0] = '!'; break;
	}
	char a[4] = "---";
	int umask = (mask >> 6);
	int gmask = (mask ^ (umask << 6)) >> 3;
	int omask = ((mask ^ ((mask >> 3) << 3)));
	getFormattedPermissionBlock(umask, a);
	out[1] = a[0]; out[2] = a[1]; out[3] = a[2];
	getFormattedPermissionBlock(gmask, a);
	out[4] = a[0]; out[5] = a[1]; out[6] = a[2];
	getFormattedPermissionBlock(omask, a);
	out[7] = a[0]; out[8] = a[1]; out[9] = a[2];
}

void formatDateTime(uint32_t time, char * out)
{
	time_t tt = time;
	struct tm * tmepoch = gmtime(&tt);
	char tstr[13] = "            ";
	strftime(tstr, 13, "    %d %H:%M", tmepoch);
	// Windows doesn't like %b for some reason...
	switch (tmepoch->tm_mon)
	{
		case 0:
			tstr[0] = 'J'; tstr[1] = 'a'; tstr[2] = 'n';
			break;
		case 1:
			tstr[0] = 'F'; tstr[1] = 'e'; tstr[2] = 'b';
			break;
		case 2:
			tstr[0] = 'M'; tstr[1] = 'a'; tstr[2] = 'r';
			break;
		case 3:
			tstr[0] = 'A'; tstr[1] = 'p'; tstr[2] = 'r';
			break;
		case 4:
			tstr[0] = 'M'; tstr[1] = 'a'; tstr[2] = 'y';
			break;
		case 5:
			tstr[0] = 'J'; tstr[1] = 'u'; tstr[2] = 'n';
			break;
		case 6:
			tstr[0] = 'J'; tstr[1] = 'u'; tstr[2] = 'l';
			break;
		case 7:
			tstr[0] = 'A'; tstr[1] = 'u'; tstr[2] = 'g';
			break;
		case 8:
			tstr[0] = 'S'; tstr[1] = 'e'; tstr[2] = 'p';
			break;
		case 9:
			tstr[0] = 'O'; tstr[1] = 'c'; tstr[2] = 't';
			break;
		case 10:
			tstr[0] = 'N'; tstr[1] = 'o'; tstr[2] = 'v';
			break;
		case 11:
			tstr[0] = 'D'; tstr[1] = 'e'; tstr[2] = 'c';
			break;
	}
	strcpy_s(out, 13, tstr);
}

void printListEntry(uint16_t id, uint16_t mask, INodeType type, uint16_t uid, uint16_t gid, uint32_t size, unsigned long mtime, char * filename)
{
	char tstr[13] = "            ";
	formatDateTime(mtime, tstr);
	char mstr[11] = "          ";
	getFormattedPermissionMask(mask, type, mstr);
	Logging::showInfoO("%10s %5d %5d %5d %9d %12s %s", mstr, id, uid, gid, size, tstr, filename);
}

int main(int argc, char* argv[])
{
	Logging::setApplicationName("testsuite");
	Logging::verbose = true;
	Logging::showInfoW("Test suite started.");

	if (!file_exists("test.afs"))
	{
#ifdef TESTSUITE_AUTOCREATE
		// Create the file
		std::fstream * fd = new std::fstream("test.afs", std::ios::out | std::ios::trunc | std::ios::in | std::ios::binary);
		if (!fd->is_open())
		{
			Logging::showErrorW("Unable to create blank AppFS package ./test.afs.");
			return 1;
		}

#if OFFSET_BOOTSTRAP != 0
#error The test suite is written under the assumption that the bootstrap offset is 0,
#error hence the test suite will not operate correctly with a different offset.
#endif
		Logging::showInfoW("AppFS test package creation required.");
		// Bootstrap section
		for (int i = 0; i < LENGTH_BOOTSTRAP; i += 1)
		{
			fd->write("\0", 1);
		}
		Logging::showInfoW("Wrote bootstrap section.");

		// Inode Lookup section
		for (int i = 0; i < 65536; i += 1)
		{
			if (i == 0)
			{
				uint32_t pos = OFFSET_ROOTINODE;
				fd->write(reinterpret_cast<char *>(&pos), 4);
			}
			else
				fd->write("\0\0\0\0", 4);
		}
		Logging::showInfoW("Wrote inode lookup section.");

		time_t rtime;
		time(&rtime);

		// Write the root inode data.
		INode node(0, "", INodeType::INT_DIRECTORY);
		node.uid = 0;
		node.gid = 1000;
		node.mask = 0777;
		node.atime = rtime;
		node.mtime = rtime;
		node.ctime = rtime;
		node.parent = 0;
		node.children_count = 0;
		std::string node_rep = node.getBinaryRepresentation();
		fd->write(node_rep.c_str(), node_rep.length());
		Logging::showInfoW("Wrote root inode.");

		fd->close();
		Logging::showInfoW("Test package creation complete.");
#else
		Logging::showErrorW("The file ./test.afs was not located in the current working");
		Logging::showErrorO("directory.  Testing can not continue.");
		return 1;
#endif
	}

	std::fstream * fd = new std::fstream("test.afs", std::ios::out | std::ios::in | std::ios::binary);
	if (!fd->is_open())
	{
		Logging::showErrorW("Unable to open test package.");
		return 1;
	}
	else
		Logging::showSuccessW("Opened test package.");
	FS fs(fd);

	// TEST: Inspect the root inode.
	INode node = fs.getINodeByID(0);
	if (node.type == INodeType::INT_INVALID)
	{
		Logging::showErrorW("Root inode is invalid (node.type == INodeType::INT_INVALID).");
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	std::vector<INode> inodechildren = fs.getChildrenOfDirectory(0);

	printListHeaders(0, inodechildren.size() + 2);
	printListEntry(node.inodeid, node.mask, node.type, node.uid, node.gid, BSIZE_DIRECTORY, node.mtime, ".");
	printListEntry(node.inodeid, node.mask, node.type, node.uid, node.gid, BSIZE_DIRECTORY, node.mtime, "..");
	for (uint16_t i = 0; i < inodechildren.size(); i += 1)
	{
		printListEntry(inodechildren[i].inodeid,
						inodechildren[i].mask,
						inodechildren[i].type,
						inodechildren[i].uid,
						inodechildren[i].gid,
						inodechildren[i].dat_len,
						inodechildren[i].mtime,
						inodechildren[i].filename);
	}

	// TEST: Locate the next available inode number.
	uint16_t inodeid = fs.getFirstFreeInodeNumber();
	if (inodeid == 0)
	{
		Logging::showErrorW("There are no free inodes available.");
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}
	Logging::showInfoW("Next free inode number is: %i", inodeid);

	// TEST: Check to see if a specified filename already exists
	//       in the root directory.
	if (fs.filenameIsUnique(0, "file") != FSResult::E_SUCCESS)
	{
		Logging::showWarningW("The file 'file' already exists in the root directory.");
	}

	// TEST: Add a new file inode.
	uint32_t pos = fs.getFirstFreeBlock(INodeType::INT_FILE);
	Logging::showInfoW("Next free data block for a file is at: %i", pos);
	time_t ltime;
	time(&ltime);
	INode node2(inodeid, "file", INodeType::INT_FILE, 1000, 1000, 0725, ltime, ltime, ltime);
	if (fs.writeINode(pos, node2) == FSResult::E_SUCCESS)
		Logging::showSuccessW("Wrote new file inode to: %i", pos);
	else
	{
		Logging::showErrorW("Unable to write new file inode to: %i", pos);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Add the new file to the root directory.
	if (fs.addChildToDirectoryInode(0, inodeid) == FSResult::E_SUCCESS)
		Logging::showSuccessW("Added child inode %i to root inode.", inodeid);
	else
	{
		Logging::showErrorW("Unable to add child inode %i to root inode.", pos);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Fetch a list of entries in the root inode.
	inodechildren = fs.getChildrenOfDirectory(0);

	printListHeaders(0, inodechildren.size() + 2);
	printListEntry(node.inodeid, node.mask, node.type, node.uid, node.gid, BSIZE_DIRECTORY, node.mtime, ".");
	printListEntry(node.inodeid, node.mask, node.type, node.uid, node.gid, BSIZE_DIRECTORY, node.mtime, "..");
	for (uint16_t i = 0; i < inodechildren.size(); i += 1)
	{
		printListEntry(inodechildren[i].inodeid,
						inodechildren[i].mask,
						inodechildren[i].type,
						inodechildren[i].uid,
						inodechildren[i].gid,
						inodechildren[i].dat_len,
						inodechildren[i].mtime,
						inodechildren[i].filename);
	}

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
	Logging::showInfoW("Test data to be written is %i bytes long.", test_data.length());
	FSResult res = fs.setFileContents(node2.inodeid, test_data.c_str(), test_data.length());
	if (res == FSResult::E_SUCCESS)
		Logging::showSuccessW("Wrote file contents for inode %i.", node2.inodeid);
	else
	{
		Logging::showErrorW("Unable to write file contents for inode %i.", node2.inodeid);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Read file contents.
	char ** data = (char**)malloc(sizeof(char*));
	uint32_t * len = (uint32_t*)malloc(sizeof(uint32_t));
	FSResult res2 = fs.getFileContents(node2.inodeid, data, len, 2048);
	if (res2 == FSResult::E_SUCCESS && data != NULL && len != NULL)
	{
		Logging::showSuccessW("Read file contents for inode %i:", node2.inodeid);
		bool matches = true;
//		Logging::showInfoW("The file contents is as follows:");
		for (uint32_t i = 0; i < *len; i += 1)
		{
			if ((*data)[i] != test_data[i])
			{
				matches = false;
				break;
			}
/*
			// Don't really like including <iostream> just to output single characters
			// but couldn't find a function which allows outputting single characters to
			// standard output without using \0 as string terminators.
			if ((*data)[i] == '\0')
				std::cout << "\\0";
			else
				std::cout << ((*data)[i]);
*/
		}
		if (matches)
			Logging::showSuccessW("File contents matches what was written.");
		else
		{
			Logging::showErrorW("File contents does not match what was written.");
			fs.close();
			Logging::showInfoW("Closed test package.");
			return 1;
		}
	}
	else
	{
		Logging::showErrorW("Unable to read file contents for inode %i.", node2.inodeid);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Overwrite data in an existing file (overlapping multiple segments).
	// We're going to change 512 bytes at an offset of 20 bytes within both the
	// test_data memory, and the new file we created (one byte at a time, stream)
	FSFile f = fs.getFile(node2.inodeid);
	f.open();
	f.seekp(20);
	const char * c = "1";
	char c2 = '1';
	for (int i = 0; i < 512; i += 1)
	{
		f.write(c, 1);
		test_data[i + 20] = c2;
	}
	f.close();
	if (f.good())
		Logging::showSuccessW("Stream writing for file was successful.");
	else
	{
		Logging::showErrorW("Unable to stream write new data into the file.");
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Confirm success of stream writing.
	data = (char**)malloc(sizeof(char*));
	len = (uint32_t*)malloc(sizeof(uint32_t));
	res2 = fs.getFileContents(node2.inodeid, data, len, 2048);
	if (res2 == FSResult::E_SUCCESS && data != NULL && len != NULL)
	{
		Logging::showSuccessW("Read file contents for inode %i:", node2.inodeid);
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
			Logging::showSuccessW("File contents matches what was written.");
		else
		{
			Logging::showErrorW("File contents does not match what was written.");
			fs.close();
			Logging::showInfoW("Closed test package.");
			return 1;
		}
	}
	else
	{
		Logging::showErrorW("Unable to read file contents for inode %i.", node2.inodeid);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Overwrite data in an existing file (overlapping multiple segments).
	// We're going to change 492 bytes at an offset of 30 bytes within both the
	// test_data memory, and the new file we created (all at once, burst)
	char * data2 = (char*)malloc(493);
	for (int i = 0; i < 492; i += 1)
	{
		data2[i] = '2';
		test_data[i + 30] = '2';
	}
	f = fs.getFile(node2.inodeid);
	f.open();
	f.seekp(30);
	f.write(data2, 492);
	f.close();
	if (f.good())
		Logging::showSuccessW("Burst writing for file was successful.");
	else
	{
		Logging::showErrorW("Unable to burst write new data into the file.");
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Confirm success of burst writing.
	data = (char**)malloc(sizeof(char*));
	len = (uint32_t*)malloc(sizeof(uint32_t));
	res2 = fs.getFileContents(node2.inodeid, data, len, 2048);
	if (res2 == FSResult::E_SUCCESS && data != NULL && len != NULL)
	{
		Logging::showSuccessW("Read file contents for inode %i:", node2.inodeid);
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
			Logging::showSuccessW("File contents matches what was written.");
		else
		{
			Logging::showErrorW("File contents does not match what was written.");
			fs.close();
			Logging::showInfoW("Closed test package.");
			return 1;
		}
	}
	else
	{
		Logging::showErrorW("Unable to read file contents for inode %i.", node2.inodeid);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	fs.close();
	Logging::showInfoW("Closed test package.");
	return 0;
}