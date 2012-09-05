/* vim: set ts=4 sw=4 tw=0 :*/

#include <exception>
#include <libapp/fs.h>
#include <libapp/exception/package.h>
#include <linux/kdev_t.h>

namespace AppLib
{
    FS::FS(std::string path)
    {
        this->stream = new LowLevel::BlockStream(path.c_str());
        if (!this->stream->is_open())
        {
            delete this->stream;
            throw Exception::PackageNotFound();
        }
        this->filesystem = new LowLevel::FS(this->stream);
        if (!this->filesystem->isValid())
        {
            this->stream->close();
            delete this->stream;
            delete this->filesystem;
            throw Exception::PackageNotValid();
        }
    }

    void FS::getattr(std::string path, struct stat& stbufOut)
        const throw(Exception::PathNotValid, Exception::FileNotFound,
                Exception::InternalInconsistency)
    {
        if (!this->checkPathIsValid(path))
            throw Exception::PathNotValid();

        LowLevel::INode buf;
        if (!this->retrievePathToINode(path, buf))
            throw Exception::FileNotFound();

        // Ensure that the inode is also one of the
        // accepted types.
        if (buf.type != LowLevel::INodeType::INT_DIRECTORY &&
                buf.type != LowLevel::INodeType::INT_FILEINFO &&
                buf.type != LowLevel::INodeType::INT_SYMLINK &&
                buf.type != LowLevel::INodeType::INT_DEVICE &&
                buf.type != LowLevel::INodeType::INT_HARDLINK)
            throw Exception::FileNotFound();

        // Resolve hardlink if needed.
        if (buf.type == LowLevel::INodeType::INT_HARDLINK)
            buf = buf.resolve(this->filesystem);

        // Set the values into the stat structure.
        stbufOut.st_ino = buf.inodeid;
        stbufOut.st_dev = buf.dev;
        stbufOut.st_mode = buf.mask;
        stbufOut.st_nlink = buf.nlink;
        stbufOut.st_uid = buf.uid;
        stbufOut.st_gid = buf.gid;
        stbufOut.st_rdev = buf.rdev;

        // File-based inodes are treated differently to directory inodes.
        if (buf.type == LowLevel::INodeType::INT_FILEINFO ||
                buf.type == LowLevel::INodeType::INT_SYMLINK ||
                buf.type == LowLevel::INodeType::INT_DEVICE)
        {
            stbufOut.st_size = buf.dat_len;
            stbufOut.st_blksize = BSIZE_FILE;
            stbufOut.st_blocks = buf.blocks;

            if (buf.type == LowLevel::INodeType::INT_FILEINFO)
                stbufOut.st_mode = S_IFREG | stbufOut.st_mode;
            else if (buf.type == LowLevel::INodeType::INT_SYMLINK)
                stbufOut.st_mode = S_IFLNK | stbufOut.st_mode;
        }
        else if (buf.type == LowLevel::INodeType::INT_DIRECTORY)
        {
            stbufOut.st_size = BSIZE_DIRECTORY;
            stbufOut.st_blksize = BSIZE_FILE;
            stbufOut.st_blocks = BSIZE_DIRECTORY / BSIZE_FILE;
            stbufOut.st_mode = S_IFDIR | stbufOut.st_mode;
        }
        else
            throw Exception::InternalInconsistency();
    }

    FSFile* FS::open(std::string path)
    {
        // TODO: Implement this function.
        return NULL;
    }

    /**** PRIVATE METHODS ****/

    bool FS::checkPathExists(std::string path) const throw()
    {
        std::vector<std::string> components = this->filesystem->splitPathBySeperators(path);
        LowLevel::INode buf = this->filesystem->getINodeByID(0);
        for (unsigned int i = 0; i < components.size(); i++)
        {
            buf = this->filesystem->getChildOfDirectory(buf.inodeid, components[i]);
            if (buf.type == LowLevel::INodeType::INT_INVALID)
                return false;
        }
        if (buf.type == LowLevel::INodeType::INT_INVALID)
            return false;
        return true;
    }

    bool FS::checkPathIsValid(std::string path) const throw()
    {
        std::vector<std::string> components = this->filesystem->splitPathBySeperators(path);
        LowLevel::FSResult::FSResult res = this->filesystem->verifyPath(path, components);
        if (res != LowLevel::FSResult::E_SUCCESS)
            return false;
        return true;
    }

    bool FS::checkPathRenamability(std::string path, uid_t uid) const throw()
    {
        // If the path doesn't exist, we don't need to check the special
        // sticky conditions.
        if (!this->checkPathExists(path))
            return false;

        // So the path does exist, and we need to check the modes on the
        // owning directory and the file.
        LowLevel::INode child, parent;
        if (!this->retrievePathToINode(path, child))
            return false;
        if (!this->retrieveParentPathToINode(path, parent))
            return false;
        if ((parent.mask & S_ISVTX) && !(child.uid == uid || parent.uid == uid))
            return false;

        // Otherwise, the user is permitted to do this.
        return true;
    }

    bool FS::checkPermission(std::string path, char op, uid_t uid, gid_t gid) const throw()
    {
        // FIXME: Implement this function.
        return true;
    }

    bool FS::retrievePathToINode(std::string path, LowLevel::INode& out, int limit) const throw()
    {
        std::vector<std::string> components = this->filesystem->splitPathBySeperators(path);
        out = this->filesystem->getINodeByID(0);
        for (unsigned int i = 0; i < (limit <= 0 ? components.size() - limit : limit); i++)
        {
            out = this->filesystem->getChildOfDirectory(out.inodeid, components[i]);
            if (out.type == LowLevel::INodeType::INT_INVALID)
                return false;
        }
        if (out.type == LowLevel::INodeType::INT_INVALID)
            return false;
        if (out.type == LowLevel::INodeType::INT_HARDLINK)
            out = out.resolve(this->filesystem);
        return true;
    }

    bool FS::retrieveParentPathToINode(std::string path, LowLevel::INode& out) const throw()
    {
        return this->retrievePathToINode(path, out, -1);
    }

    void FS::saveINode(LowLevel::INode& buf)
        throw(Exception::INodeSaveInvalid, Exception::INodeSaveFailed)
    {
        if (buf.type == LowLevel::INodeType::INT_INVALID ||
                buf.type == LowLevel::INodeType::INT_UNSET)
            throw Exception::INodeSaveInvalid();
        if (this->filesystem->updateINode(buf) != LowLevel::FSResult::E_SUCCESS)
            throw Exception::INodeSaveFailed();
    }

    void FS::saveNewINode(uint32_t pos, LowLevel::INode& buf)
        throw(Exception::INodeSaveInvalid, Exception::INodeSaveFailed)
    {
        if (buf.type == LowLevel::INodeType::INT_INVALID ||
                buf.type == LowLevel::INodeType::INT_UNSET)
            throw Exception::INodeSaveInvalid();
        if (this->filesystem->writeINode(pos, buf) != LowLevel::FSResult::E_SUCCESS)
            throw Exception::INodeSaveFailed();
    }

    int FS::extractMaskFromMode(mode_t mode) const throw()
    {
        if (mode & S_IFDIR)
            return mode & ~S_IFDIR;
        else if (mode & S_IFLNK)
            return mode & ~S_IFLNK;
        else if (mode & S_IFREG)
            return mode & ~S_IFREG;
        else
            return mode; // Keep other information.
    }

    std::string FS::extractBasenameFromPath(std::string path) const throw()
    {
        std::vector<std::string> components = this->filesystem->splitPathBySeperators(path);
        if (components.size() == 0)
            return "";
        return components[components.size() - 1];
    }

    LowLevel::INode FS::assignNewINode(LowLevel::INodeType::INodeType type, uint32_t& posOut)
        throw(Exception::INodeSaveInvalid, Exception::INodeSaveFailed,
                Exception::NoFreeSpace, Exception::INodeExhaustion)
    {
        if (type == LowLevel::INodeType::INT_INVALID ||
                type == LowLevel::INodeType::INT_UNSET)
            throw Exception::INodeSaveInvalid();
        
        // Determine appropriate type.
        LowLevel::INodeType::INodeType simple = LowLevel::INodeType::INT_FILEINFO;
        if (type == LowLevel::INodeType::INT_DIRECTORY)
            simple = type;

        // Get a free block.
        posOut = this->filesystem->getFirstFreeBlock(simple);
        if (posOut == 0)
            throw Exception::NoFreeSpace();

        // Get a free inode number.
        uint16_t id = this->filesystem->getFirstFreeINodeNumber();
        if (id == 0)
            throw Exception::INodeExhaustion();
        LowLevel::INode buf;
        buf.inodeid = id;

        // Ensure that we don't have an invalid position and that the
        // inode ID is not already assigned.
        if (posOut < OFFSET_DATA)
            throw Exception::INodeSaveInvalid();
        if (type != LowLevel::INodeType::INT_SEGINFO &&
                type != LowLevel::INodeType::INT_FREELIST &&
                this->filesystem->getINodePositionByID(id) != 0)
            throw Exception::INodeSaveInvalid();

        // Reserve the inode ID with this INode's position.
        this->filesystem->reserveINodeID(id);

        // Return the new INode.  If any future operation fails before
        // this inode is written to disk, it is the callee's responsibility
        // to ensure the ID is freed.
        return buf;
    }
}
