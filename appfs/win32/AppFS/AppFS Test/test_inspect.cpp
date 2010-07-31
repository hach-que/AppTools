#include "test_global.h"

int test_inspect(AppLib::LowLevel::FS * fs, uint16_t inodeid)
{
	AppLib::LowLevel::INode node = fs->getINodeByID(inodeid);
	if (node.type == AppLib::LowLevel::INodeType::INT_INVALID)
	{
		AppLib::Logging::showErrorW("INode with ID %i is invalid (node.type == INodeType::INT_INVALID).",
			inodeid);
		return 1;
	}

	std::vector<AppLib::LowLevel::INode> inodechildren = fs->getChildrenOfDirectory(inodeid);

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

	return 0;
}