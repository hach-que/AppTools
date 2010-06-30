// AppFS Test.cpp : Defines the entry point for the console application.
//
#include "logging.h"
#include "fs.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <vector>

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

void formatDateTime(unsigned long time, char * out)
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

void printListEntry(uint16_t id, char * mask, uint16_t uid, uint16_t gid, unsigned long mtime, char * filename)
{
	char tstr[13] = "            ";
	formatDateTime(mtime, tstr);
	Logging::showInfoO("%10s %5d %5d %5d %9d %12s %s", mask, id, uid, gid, 0, tstr, filename);
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
				unsigned long pos = OFFSET_ROOTINODE;
				fd->write(reinterpret_cast<char *>(&pos), 4);
			}
			else
				fd->write("\0\0\0\0", 4);
		}
		Logging::showInfoW("Wrote inode lookup section.");

		// Write the root inode data.
		INode node(0, "", INodeType::INT_DIRECTORY);
		node.uid = 1000;
		node.gid = 1000;
		node.mask = 0777;
		node.atime = 1277635895;
		node.mtime = 1277635895;
		node.ctime = 1277635895;
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
		Logging::showInfoW("Opened test package.");
	FS fs(fd);

	// TEST: Inspect the root inode.
	printListHeaders(0, 2);
	INode node = fs.getINodeByID(0);
	if (node.type == INodeType::INT_INVALID)
	{
		Logging::showErrorW("Root inode is invalid (node.type == INodeType::INT_INVALID).");
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	printListEntry(node.inodeid, "drwxrwxrwx", node.uid, node.gid, node.mtime, ".");
	printListEntry(node.inodeid, "drwxrwxrwx", node.uid, node.gid, node.mtime, "..");

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

	// TEST: Add a new file inode.
	unsigned long pos = fs.getFirstFreeBlock(INodeType::INT_FILE);
	Logging::showInfoW("Next free data block for a file is at: %i", pos);
	INode node2(inodeid, "file", INodeType::INT_FILE, 1001, 1001, 0755, 1000000000, 1000000000, 1000000000);
	if (fs.writeINode(pos, node2) == FSResult::E_SUCCESS)
		Logging::showInfoW("Wrote new file inode to: %i", pos);
	else
	{
		Logging::showErrorW("Unable to write new file inode to: %i", pos);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Get the root inode again and show it's children.
	if (fs.addChildToDirectoryInode(0, inodeid) == FSResult::E_SUCCESS)
		Logging::showInfoW("Added child inode %i to root inode.", inodeid);
	else
	{
		Logging::showErrorW("Unable to add child inode %i to root inode.", pos);
		fs.close();
		Logging::showInfoW("Closed test package.");
		return 1;
	}

	// TEST: Fetch a list of entries in the root inode.
	std::vector<INode> inodechildren = fs.getChildrenOfDirectory(0);

	printListHeaders(0, inodechildren.size() + 2);
	printListEntry(node.inodeid, "drwxrwxrwx", node.uid, node.gid, node.mtime, ".");
	printListEntry(node.inodeid, "drwxrwxrwx", node.uid, node.gid, node.mtime, "..");
	for (uint16_t i = 0; i < inodechildren.size(); i += 1)
	{
		printListEntry(inodechildren[i].inodeid,
						"drwxrwxrwx",
						inodechildren[i].uid,
						inodechildren[i].gid,
						inodechildren[i].mtime,
						inodechildren[i].filename);
	}

	// TODO: Add function for reading the actual children from an inode.

	// TODO: Add test for adding new entries to the root inode (and then listing
	//       the contents of the children).

	fs.close();
	Logging::showInfoW("Closed test package.");
	return 0;
}