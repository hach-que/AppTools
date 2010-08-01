#include "global.h"

bool file_exists(std::string filename)
{
	struct stat file_info;
	return (stat(filename.c_str(), &file_info) == 0);
}

void printListHeaders(uint16_t id, uint16_t total)
{
	AppLib::Logging::showInfoW("inode %i, total %i:", id, total);
	AppLib::Logging::showInfoO("MASK       NODID   UID   GID      SIZE DATETIME     FNAME");
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

void getFormattedPermissionMask(uint16_t mask, AppLib::LowLevel::INodeType::INodeType type, char * out)
{
	for (int i = 0; i < 11; i += 1)
		out[i] = '\0';
	out[0] = '-';
	switch (type)
	{
		case AppLib::LowLevel::INodeType::INT_FREEBLOCK:	out[0] = '_'; break;
		case AppLib::LowLevel::INodeType::INT_FILE:		out[0] = '-'; break;
		case AppLib::LowLevel::INodeType::INT_DIRECTORY:	out[0] = 'd'; break;
		case AppLib::LowLevel::INodeType::INT_SEGMENT:	out[0] = '#'; break;
		case AppLib::LowLevel::INodeType::INT_INVALID:	out[0] = '!'; break;
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

void printListEntry(uint16_t id, uint16_t mask, AppLib::LowLevel::INodeType::INodeType type, uint16_t uid, uint16_t gid, uint32_t size, unsigned long mtime, char * filename)
{
	char tstr[13] = "            ";
	formatDateTime(mtime, tstr);
	char mstr[11] = "          ";
	getFormattedPermissionMask(mask, type, mstr);
	AppLib::Logging::showInfoO("%10s %5d %5d %5d %9d %12s %s", mstr, id, uid, gid, size, tstr, filename);
}

const char* getFSResultName(AppLib::LowLevel::FSResult::FSResult res)
{
	switch (res)
	{
		case AppLib::LowLevel::FSResult::E_SUCCESS:
			return "E_SUCCESS";
		case AppLib::LowLevel::FSResult::E_FAILURE_GENERAL:
			return "E_FAILURE_GENERAL";
		case AppLib::LowLevel::FSResult::E_FAILURE_INODE_ALREADY_ASSIGNED:
			return "E_FAILURE_INODE_ALREADY_ASSIGNED";
		case AppLib::LowLevel::FSResult::E_FAILURE_INODE_NOT_ASSIGNED:
			return "E_FAILURE_INODE_NOT_ASSIGNED";
		case AppLib::LowLevel::FSResult::E_FAILURE_INODE_NOT_VALID:
			return "E_FAILURE_INODE_NOT_VALID";
		case AppLib::LowLevel::FSResult::E_FAILURE_INVALID_FILENAME:
			return "E_FAILURE_INVALID_FILENAME";
		case AppLib::LowLevel::FSResult::E_FAILURE_INVALID_POSITION:
			return "E_FAILURE_INVALID_POSITION";
		case AppLib::LowLevel::FSResult::E_FAILURE_MAXIMUM_CHILDREN_REACHED:
			return "E_FAILURE_MAXIMUM_CHILDREN_REACHED";
		case AppLib::LowLevel::FSResult::E_FAILURE_NOT_A_DIRECTORY:
			return "E_FAILURE_NOT_A_DIRECTORY";
		case AppLib::LowLevel::FSResult::E_FAILURE_NOT_A_FILE:
			return "E_FAILURE_NOT_A_FILE";
		case AppLib::LowLevel::FSResult::E_FAILURE_NOT_IMPLEMENTED:
			return "E_FAILURE_NOT_IMPLEMENTED";
		case AppLib::LowLevel::FSResult::E_FAILURE_NOT_UNIQUE:
			return "E_FAILURE_NOT_UNIQUE";
		case AppLib::LowLevel::FSResult::E_FAILURE_PARTIAL_TRUNCATION:
			return "E_FAILURE_PARTIAL_TRUNCATION";
		case AppLib::LowLevel::FSResult::E_FAILURE_UNKNOWN:
			return "E_FAILURE_UNKNOWN";
		default:
			return "<Unknown>";
	}
}