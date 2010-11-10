// actions.cpp : Defines the actions for the command line.
//

#include "main.h"
#include <conio.h>
#include <errno.h>
#include <time.h>

void doOpen(std::vector<std::string> cmd)
{
	if (packageIsOpen())
	{
		std::cout << "There is a package already open!" << std::endl;
		return;
	}
	if (!checkArguments("open", cmd, 1)) return;

	if (!fileExists(cmd[1]))
	{
		std::cout << "The specified filename or directory could not be found." << std::endl;
		return;
	}

	std::cout << "Attempting to open '" << cmd[1] << "' ... ";
	fd = new AppLib::LowLevel::BlockStream(cmd[1]);
	if (!fd->is_open())
	{
		std::cout << "failed." << std::endl;
		std::cout << "Unable to open: " << cmd[1] << std::endl;
		std::cout << "Check the file exists, is writable and try again." << std::endl;
		return;
	}
	else
		std::cout << "done." << std::endl;

	currentFilesystem = new AppLib::LowLevel::FS(fd);
	if (!currentFilesystem->isValid())
	{
		std::cout << "Unable to read file as AppFS package.  Aborting open." << std::endl;
		delete currentFilesystem;
		currentFilesystem = NULL;
		fd->close();
		delete fd;
		fd = NULL;
		return;
	}
	openPackage = cmd[1];
	currentPath = "/";
	showCurrentOpenPackage();
}

void doCreate(std::vector<std::string> cmd)
{
	if (packageIsOpen())
	{
		std::cout << "There is a package already open!" << std::endl;
		return;
	}
	if (!checkArguments("create", cmd, 1)) return;

	if (fileExists(cmd[1]))
	{
		std::cout << "The specified filename or directory already exists." << std::endl;
		return;
	}

	std::cout << "Attempting to create '" << cmd[1] << "' ... " << std::endl;

	// Create the file.
	if (!AppLib::LowLevel::Util::createPackage(cmd[1], "Test App", "1.0.0", "A test package.", "AppTools"))
	{
		std::cout << "Unable to create blank AppFS package '" << cmd[1] << "'." << std::endl;
		return;
	}
	std::cout << "Package successfully created.  Use 'open' to open the package." << std::endl;
}

void doCd(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("cd", cmd, 1)) return;
	bool isRelativeToRoot = (cmd[1][0] == '/');
	std::string fullPath = "";
	if (isRelativeToRoot)
		fullPath = cmd[1];
	else
		fullPath = combinePaths(currentPath, cmd[1]);

	// Check if the specified path exists.
	int32_t id = currentFilesystem->resolvePathnameToInodeID(sanitizeAbsolutePath(fullPath).c_str());
	if (id == -ENOENT)
	{
		std::cout << "The specified filename or directory could not be found." << std::endl;
		return;
	}

	currentPath = sanitizeAbsolutePath(fullPath);
}

void doLs(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("ls", cmd, 0, 1)) return;

	int32_t id = 0;
	if (cmd.size() > 1)
	{
		// The user supplied a path.
		id = currentFilesystem->resolvePathnameToInodeID(combinePaths(currentPath, cmd[1]).c_str());
	}
	else
	{
		id = currentFilesystem->resolvePathnameToInodeID(currentPath.c_str());
	}
	if (id == -ENOENT)
	{
		std::cout << "The specified filename or directory could not be found." << std::endl;
		return;
	}

	AppLib::LowLevel::INode node = currentFilesystem->getINodeByID(id);
	std::vector<AppLib::LowLevel::INode> inodechildren = currentFilesystem->getChildrenOfDirectory(id);

	printListHeaders(0, inodechildren.size() + 2);
	printListEntry(node.inodeid, node.mask, node.type, node.uid, node.gid, BSIZE_DIRECTORY, node.mtime, ".");
	printListEntry(node.inodeid, node.mask, node.type, node.uid, node.gid, BSIZE_DIRECTORY, node.mtime, "..");
	for (uint16_t i = 0; i < inodechildren.size(); i += 1)
	{
		if (inodechildren[i].type == AppLib::LowLevel::INodeType::INT_DIRECTORY)
			inodechildren[i].dat_len = BSIZE_DIRECTORY;
		printListEntry(inodechildren[i].inodeid,
						inodechildren[i].mask,
						inodechildren[i].type,
						inodechildren[i].uid,
						inodechildren[i].gid,
						inodechildren[i].dat_len,
						inodechildren[i].mtime,
						inodechildren[i].filename);
	}
}

void doMkdir(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("mkdir", cmd, 1)) return;
	if (cmd[1].find("/") != std::string::npos)
	{
		std::cout << "Please cd to the location you wish to create the directory in" << std::endl;
		std::cout << "instead of providing a relative path." << std::endl;
		return;
	}

	// Resolve current directory path.
	int32_t id = currentFilesystem->resolvePathnameToInodeID(currentPath.c_str());
	if (id < 0)
	{
		std::cout << "Unable to resolve current directory path to inode." << std::endl;
		return;
	}

	// Get the next free inode id.
	uint16_t newid = currentFilesystem->getFirstFreeInodeNumber();

	// Create the directory.
	uint32_t pos = currentFilesystem->getFirstFreeBlock(AppLib::LowLevel::INodeType::INT_DIRECTORY);
	time_t ltime; time(&ltime);
	AppLib::LowLevel::INode node(newid, const_cast<char*>(cmd[1].c_str()), AppLib::LowLevel::INodeType::INT_DIRECTORY, 1000, 1000, 0755, ltime, ltime, ltime);
	node.parent = id;
	if (currentFilesystem->writeINode(pos, node) != AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		std::cout << "Unable to create directory." << std::endl;
		return;
	}

	// Add the directory to the current inode id.
	if (currentFilesystem->addChildToDirectoryInode(id, newid) == AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		std::cout << "Successfully created directory." << std::endl;
		return;
	}
	else
	{
		// Failed, reset the block we used for the directory inode.
		for (int i = 0; i < BSIZE_DIRECTORY / BSIZE_FILE; i += 1)
			currentFilesystem->resetBlock(pos + (512 * i));

		std::cout << "Unable to add directory inode to current directory." << std::endl;
		return;
	}
}

void doTouch(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("touch", cmd, 1)) return;
	if (cmd[1].find("/") != std::string::npos)
	{
		std::cout << "Please cd to the location you wish to create the file in" << std::endl;
		std::cout << "instead of providing a relative path." << std::endl;
		return;
	}

	// Resolve current directory path.
	int32_t id = currentFilesystem->resolvePathnameToInodeID(currentPath.c_str());
	if (id < 0)
	{
		std::cout << "Unable to resolve current directory path to inode." << std::endl;
		return;
	}

	// Check to see if file already exists.
	if (currentFilesystem->filenameIsUnique(id, const_cast<char*>(cmd[1].c_str())) == AppLib::LowLevel::FSResult::E_FAILURE_NOT_UNIQUE)
	{
		std::cout << "Unable to create file: filename is not unique." << std::endl;
		return;
	}

	// Get the next free inode id.
	uint16_t newid = currentFilesystem->getFirstFreeInodeNumber();

	// Create the file.
	uint32_t pos = currentFilesystem->getFirstFreeBlock(AppLib::LowLevel::INodeType::INT_FILEINFO);
	time_t ltime; time(&ltime);
	AppLib::LowLevel::INode node(newid, const_cast<char*>(cmd[1].c_str()), AppLib::LowLevel::INodeType::INT_FILEINFO, 1000, 1000, 0311, ltime, ltime, ltime);
	node.parent = id;
	if (currentFilesystem->writeINode(pos, node) != AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		std::cout << "Unable to create file." << std::endl;
		return;
	}

	// Add the file to the current inode id.
	if (currentFilesystem->addChildToDirectoryInode(id, newid) == AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		std::cout << "Successfully created file." << std::endl;
		return;
	}
	else
	{
		// Failed, reset the block we used for the file inode.
		currentFilesystem->resetBlock(pos);

		std::cout << "Unable to add directory inode to current directory." << std::endl;
		return;
	}
}

void doRm(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("rm", cmd, 1)) return;
	if (cmd[1].find("/") != std::string::npos)
	{
		std::cout << "Please cd to the location you wish to remove the file in" << std::endl;
		std::cout << "instead of providing a relative path." << std::endl;
		return;
	}

	// Resolve current directory path.
	int32_t id = currentFilesystem->resolvePathnameToInodeID(currentPath.c_str());
	if (id < 0)
	{
		std::cout << "Unable to resolve current directory path to inode." << std::endl;
		return;
	}

	// Get the inode of the file.
	AppLib::LowLevel::INode fnode = currentFilesystem->getChildOfDirectory(id, cmd[1].c_str());
	if (fnode.type == AppLib::LowLevel::INodeType::INT_INVALID)
	{
		std::cout << "The specified filename or directory could not be found." << std::endl;
		return;
	}

	// Erase all of the file segments, except for the first entry.
	uint32_t npos = currentFilesystem->getFileNextBlock(
		fnode.inodeid, 
		currentFilesystem->getINodePositionByID(fnode.inodeid));
	uint32_t opos = npos;
	while (npos != 0)
	{
		npos = currentFilesystem->getFileNextBlock(fnode.inodeid, npos);
		currentFilesystem->resetBlock(opos);
		opos = npos;
	}

	// Now remove the entry from the parent directory.
	if (currentFilesystem->removeChildFromDirectoryInode(id, fnode.inodeid) != AppLib::LowLevel::FSResult::E_SUCCESS)
	{
		std::cout << "Unable to remove the child listing in the parent directory." << std::endl;
		return;
	}

	// Delete the first block.
	currentFilesystem->resetBlock(currentFilesystem->getINodePositionByID(fnode.inodeid));

	std::cout << "Successfully removed file." << std::endl;
	return;
}

void doTruncate(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("truncate", cmd, 2)) return;
	if (cmd[1].find("/") != std::string::npos)
	{
		std::cout << "Please cd to the location you wish to truncate the file in" << std::endl;
		std::cout << "instead of providing a relative path." << std::endl;
		return;
	}
	int flength = atoi(cmd[2].c_str());
	if (flength <= 0)
	{
		std::cout << "Specified file length is not a number or is invalid (<= 0)." << std::endl;
		return;
	}

	// Resolve current directory path.
	int32_t id = currentFilesystem->resolvePathnameToInodeID(currentPath.c_str());
	if (id < 0)
	{
		std::cout << "Unable to resolve current directory path to inode." << std::endl;
		return;
	}

	// Get the inode of the file.
	AppLib::LowLevel::INode fnode = currentFilesystem->getChildOfDirectory(id, cmd[1].c_str());
	if (fnode.type == AppLib::LowLevel::INodeType::INT_INVALID)
	{
		std::cout << "The specified filename or directory could not be found." << std::endl;
		return;
	}
	if (fnode.type != AppLib::LowLevel::INodeType::INT_FILEINFO)
	{
		std::cout << "The specified filename is not a file." << std::endl;
		return;
	}

	// Truncate the file.
	AppLib::LowLevel::FSResult::FSResult res = currentFilesystem->truncateFile(fnode.inodeid, flength);

	if (res == AppLib::LowLevel::FSResult::E_SUCCESS)
		std::cout << "Successfully truncated file." << std::endl;
	else if (res == AppLib::LowLevel::FSResult::E_FAILURE_PARTIAL_TRUNCATION)
		std::cout << "File was partially truncated." << std::endl;
	else
		std::cout << "Unable to truncate file." << std::endl;
	return;
}

void doEdit(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	if (!checkArguments("edit", cmd, 1)) return;
	if (cmd[1].find("/") != std::string::npos)
	{
		std::cout << "Please cd to the location you wish to edit the file in" << std::endl;
		std::cout << "instead of providing a relative path." << std::endl;
		return;
	}

	// Resolve current directory path.
	int32_t id = currentFilesystem->resolvePathnameToInodeID(currentPath.c_str());
	if (id < 0)
	{
		std::cout << "Unable to resolve current directory path to inode." << std::endl;
		return;
	}

	// Get the inode of the file.
	AppLib::LowLevel::INode fnode = currentFilesystem->getChildOfDirectory(id, cmd[1].c_str());
	if (fnode.type == AppLib::LowLevel::INodeType::INT_INVALID)
	{
		std::cout << "The specified filename or directory could not be found." << std::endl;
		return;
	}

	// Read all of the file data.
	std::fstream * tfd = new std::fstream("./temp.dat", std::ios::out | std::ios::trunc);
	if (!tfd->is_open())
	{
		std::cout << "Failed to open temporary file." << std::endl;
		return;
	}

	// Allocate memory and open the file.
	char * data = (char*)malloc(513);
	for (int i = 0; i < 513; i += 1)
		data[i] = 0;
	std::streamsize rlen = 0;
	AppLib::LowLevel::FSFile filer = currentFilesystem->getFile(fnode.inodeid);
	filer.open();
	filer.seekg(0);

	// Read in 512 byte blocks and write it to a temporary file.
	rlen = filer.read(data, 512);
	if (filer.bad() || filer.fail())
	{
		std::cout << "Failed to read from file (reading first 512 bytes)." << std::endl;
		free(data);
		tfd->close();
		filer.close();
		return;
	}
	while (rlen != 0)
	{
		tfd->write(data, rlen);
		if (tfd->bad())
		{
			std::cout << "Failed to write to temporary file (writing " << rlen << " bytes)." << std::endl;
			free(data);
			tfd->close();
			filer.close();
			return;
		}
		for (int i = 0; i < 513; i += 1)
			data[i] = 0;
		if (rlen == 512)
		{
			rlen = filer.read(data, 512);
			if (filer.bad() || filer.fail())
			{
				std::cout << "Failed to read from file (reading 512 bytes)." << std::endl;
				free(data);
				tfd->close();
				filer.close();
				return;
			}
		}
		else
			break;
	}
	free(data);
	tfd->close();
	filer.close();

	// The file has now been written to a temporary file.  Run the editor.
	std::string editorline;
	editorline += "\"";
	editorline += environmentMappings["editor"];
	editorline += "\" ./temp.dat";
	system(editorline.c_str());

	// Now read in the temporary file data and write it back to the file
	// in the package.
	tfd = new std::fstream("./temp.dat", std::ios::in);
	if (!tfd->is_open())
	{
		std::cout << "Failed to open temporary file." << std::endl;
		return;
	}

	// Open the file again.
	char stor = '\0';
	AppLib::LowLevel::FSFile filew = currentFilesystem->getFile(fnode.inodeid);
	filew.open();
	filew.seekp(0);
	
	// Truncate the file.
	filew.truncate(0);
	
	// Since the iostream.read() functionality doesn't provide a way to determine
	// the number of bytes read (i.e. if there's less bytes to read than there are
	// available before EOF, there's no way to determine this other than NULL characters
	// which means you can't contain NULL characters within the temporary file), we
	// need to read one byte at a time.
	tfd->read(&stor, 1);
	if (tfd->bad() || tfd->fail())
	{
		std::cout << "Failed to read from temporary file." << std::endl;
		tfd->close();
		filew.close();
		return;
	}
	while (!tfd->fail() && !tfd->eof() && !tfd->bad())
	{
		filew.write(const_cast<const char*>(&stor), 1);
		if (filew.bad() || filew.fail())
		{
			std::cout << "Failed to write to file." << std::endl;
			tfd->close();
			filew.close();
			return;
		}
		tfd->read(&stor, 1);
		if ((tfd->bad() || tfd->fail()) && !tfd->eof())
		{
			std::cout << "Failed to read from temporary file." << std::endl;
			tfd->close();
			filew.close();
			return;
		}
	}
	tfd->close();
	filew.close();

	std::cout << "Successfully edited file." << std::endl;
	return;
}

void doEnvSet(std::vector<std::string> cmd)
{
	if (!checkArguments("envset", cmd, 2)) return;
	environmentMappings[cmd[1]] = cmd[2];
	std::cout << cmd[1] << " = " << cmd[2] << std::endl;
	return;
}

void doEnvGet(std::vector<std::string> cmd)
{
	if (!checkArguments("envget", cmd, 1)) return;
	std::cout << environmentMappings[cmd[1]] << std::endl;
	return;
}

void doClose(std::vector<std::string> cmd)
{
	if (!checkPackageOpen()) return;
	std::cout << "Closing '" << openPackage << "' ... ";
	currentFilesystem->close();
	delete currentFilesystem;
	fd->close();
	delete fd;
	fd = NULL;
	openPackage = "";
	std::cout << "done." << std::endl;
	showCurrentOpenPackage();
}