
#include <FileHandle.h>

#include <sstream>

#include <FileSys.h>
#include <FilePath.h>
#include <FileIterator.h>
#include <FileVisitor.h>
#include <FunctionalFileVisitor.h>
#include <FileWatcher.h>
#include <Tree.h>
#include <AbstractFileSystem.h>
#include <AbstractFileHandleBackend.h>
#include <AbstractFileIteratorBackend.h>


namespace FileSys
{


FileHandle::FileHandle()
: m_backend(nullptr)
{
}

FileHandle::FileHandle(eastl::unique_ptr<AbstractFileHandleBackend> && backend)
: m_backend(eastl::move(backend))
{
}

FileHandle::FileHandle(const FileHandle & fileHandle)
: m_backend(fileHandle.m_backend ? fileHandle.m_backend->clone() : nullptr)
{
}

FileHandle::FileHandle(FileHandle && fileHandle)
: m_backend(eastl::move(fileHandle.m_backend))
{
}

FileHandle::~FileHandle()
{
}

FileHandle & FileHandle::operator=(const FileHandle & fileHandle)
{
    if (fileHandle.m_backend)
    {
        m_backend = fileHandle.m_backend->clone();
    }
    else
    {
        m_backend.reset(nullptr);
    }

    return *this;
}

FileHandle & FileHandle::operator=(FileHandle && fileHandle)
{
    m_backend = eastl::move(fileHandle.m_backend);

    return *this;
}

AbstractFileSystem * FileHandle::fs() const
{
    return m_backend ? m_backend->fs() : nullptr;
}

eastl::string FileHandle::path() const
{
    return m_backend ? m_backend->path() : "";
}

eastl::string FileHandle::fileName() const
{
    return m_backend ? FilePath(m_backend->path()).fileName() : "";
}

void FileHandle::updateFileInfo()
{
    if (m_backend) m_backend->updateFileInfo();
}

bool FileHandle::exists() const
{
    return m_backend ? m_backend->exists() : false;
}

bool FileHandle::isInUse() const
{
    if (exists() && isFile())
    {
        FileStream stream = createFileStream(FileRead);
        if (!stream.isValid()) return true;
    }
    return false;
}

bool FileHandle::isFile() const
{
    return m_backend ? m_backend->isFile() : false;
}

bool FileHandle::isDirectory() const
{
    return m_backend ? m_backend->isDirectory() : false;
}

bool FileHandle::isSymbolicLink() const
{
    return m_backend ? m_backend->isSymbolicLink() : false;
}

eastl::vector<eastl::string> FileHandle::listFiles() const
{
    return m_backend ? m_backend->listFiles() : eastl::vector<eastl::string>();
}

void FileHandle::traverse(VisitFunc funcFileEntry)
{
    FunctionalFileVisitor visitor(funcFileEntry);
    traverse(visitor);
}

void FileHandle::traverse(VisitFunc funcFile, VisitFunc funcDirectory)
{
    FunctionalFileVisitor visitor(funcFile, funcDirectory);
    traverse(visitor);
}

void FileHandle::traverse(FileVisitor & visitor)
{
    // Check if file or directory exists
    if (!exists())
    {
        return;
    }

    // Invoke visitor
    bool traverseSubDir = visitor.onFileEntry(*this);

    // Is this is directory?
    if (isDirectory() && traverseSubDir)
    {
        // Iterator over child entries
        for (auto it = begin(); it != end(); ++it)
        {
            // Open file or directory
            FileHandle fh = open(*it);
            if (!fh.exists()) continue;

            // Handle entry
            fh.traverse(visitor);
        }
    }
}

eastl::unique_ptr<Tree> FileHandle::readTree(const eastl::string & path) const
{
    // Check if file or directory exists
    if (!exists())
    {
        return nullptr;
    }

    // Create tree
    auto tree = eastl::unique_ptr<Tree>(new Tree);
    tree->setPath(path);
    tree->setFileName(fileName());
    tree->setDirectory(isDirectory());
    tree->setSize(size());
    tree->setAccessTime(accessTime());
    tree->setModificationTime(modificationTime());
    tree->setUserId(userId());
    tree->setGroupId(groupId());
    tree->setPermissions(permissions());

    // Is this is directory?
    if (isDirectory())
    {
        // Add children
        for (auto it = begin(); it != end(); ++it)
        {
            // Open file or directory
            FileHandle fh = open(*it);
            if (!fh.exists()) continue;

            // Compose name
            eastl::string subName = path;
            if (!subName.empty()) subName += "/";
            subName += fh.fileName();

            // Read subtree
            auto subTree = fh.readTree(subName);

            // Add subtree to list
            if (subTree)
            {
                tree->add(eastl::move(subTree));
            }
        }
    }

    // Return tree
    return tree;
}

FileIterator FileHandle::begin() const
{
    return m_backend ? FileIterator(m_backend->begin()) : FileIterator();
}

FileIterator FileHandle::end() const
{
    return FileIterator();
}

unsigned int FileHandle::size() const
{
    return m_backend ? m_backend->size() : 0;
}

unsigned int FileHandle::accessTime() const
{
    return m_backend ? m_backend->accessTime() : 0;
}

uint64_t FileHandle::modificationTime() const
{
    return m_backend ? m_backend->modificationTime() : 0;
}

unsigned int FileHandle::userId() const
{
    return m_backend ? m_backend->userId() : 0;
}

void FileHandle::setUserId(unsigned int uid)
{
    if (m_backend) m_backend->setUserId(uid);
}

unsigned int FileHandle::groupId() const
{
    return m_backend ? m_backend->groupId() : 0;
}

void FileHandle::setGroupId(unsigned int gid)
{
    if (m_backend) m_backend->setGroupId(gid);
}

unsigned long FileHandle::permissions() const
{
    return m_backend ? m_backend->permissions() : 0;
}

void FileHandle::setPermissions(unsigned long permissions)
{
    if (m_backend) m_backend->setPermissions(permissions);
}

FileHandle FileHandle::parentDirectory() const
{
    return m_backend ? m_backend->fs()->open(FilePath(m_backend->path()).resolve("..").resolved()) : FileHandle();
}

FileHandle FileHandle::open(const eastl::string & path) const
{
    return m_backend ? m_backend->fs()->open(FilePath(m_backend->path()).resolve(path).fullPath()) : FileHandle();
}

bool FileHandle::createDirectory()
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // Make directory
    if (!m_backend->createDirectory())
    {
        return false;
    }

    // Done
    return true;
}

bool FileHandle::removeDirectory()
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // Remove directory
    if (!m_backend->removeDirectory())
    {
        return false;
    }

    // Done
    return true;
}

void FileHandle::copyDirectoryRec(FileHandle & dstDir)
{
    // Check if source directory is valid
    if (!isDirectory())
    {
        return;
    }

    // Check destination directory and try to create it if necessary
    if (!dstDir.isDirectory())
    {
        dstDir.createDirectory();

        if (!dstDir.isDirectory())
        {
            return;
        }
    }

    // Copy all entries
    for (auto it = begin(); it != end(); ++it)
    {
        eastl::string filename = *it;

        FileHandle src = this->open(filename);
        FileHandle dst = dstDir.open(filename);

        if (src.isDirectory())
        {
            src.copyDirectoryRec(dst);
        }

        else if (src.isFile())
        {
            src.copy(dst);
        }
    }
}

void FileHandle::removeDirectoryRec()
{
    // Check directory
    if (!isDirectory()) {
        return;
    }

    // Delete all entries
    for (auto it = begin(); it != end(); ++it)
    {
        FileHandle fh = open(*it);

        if (fh.isDirectory())
        {
            fh.removeDirectoryRec();
        }

        else if (fh.isFile())
        {
            fh.remove();
        }
    }

    // Remove directory
    removeDirectory();
}

bool FileHandle::copy(FileHandle & dest)
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // If both handles are from the same file system, use internal method
    if (m_backend->fs() == dest.m_backend->fs())
    {
        bool result = m_backend->copy(*dest.m_backend.get());
        dest.updateFileInfo();
        return result;
    }

    // Otherwise, use generic (slow) method
    else
    {
        return genericCopy(dest);
    }
}

bool FileHandle::move(FileHandle & dest)
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // If both handles are from the same file system, use internal method
    if (m_backend->fs() == dest.m_backend->fs())
    {
        bool result = m_backend->move(*dest.m_backend.get());
        dest.updateFileInfo();
        return result;
    }

    // Otherwise, use generic (slow) method
    else
    {
        return genericMove(dest);
    }
}

bool FileHandle::createLink(FileHandle & dest)
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // If both handles are from the same file system, use internal method
    if (m_backend->fs() == dest.m_backend->fs())
    {
        bool result = m_backend->createLink(*dest.m_backend.get());
        dest.updateFileInfo();
        return result;
    }

    // Otherwise, this is not possible
    else
    {
        return false;
    }
}

bool FileHandle::createSymbolicLink(FileHandle & dest)
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // If both handles are from the same file system, use internal method
    if (m_backend->fs() == dest.m_backend->fs())
    {
        bool result = m_backend->createSymbolicLink(*dest.m_backend.get());
        dest.updateFileInfo();
        return result;
    }

    // Otherwise, this is not possible
    else
    {
        return false;
    }
}

bool FileHandle::rename(const eastl::string & filename)
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // Rename file
    if (!m_backend->rename(filename))
    {
        return false;
    }

    // Done
    return true;
}

bool FileHandle::remove()
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // Remove file
    if (!m_backend->remove())
    {
        return false;
    }

    // Update file information
    return true;
}

FileWatcher FileHandle::watch(unsigned int events, RecursiveMode recursive)
{
    // Create file system watcher
    FileWatcher watcher(fs());
    watcher.add(*this, events, recursive);

    // Return watcher
    return std::move(watcher);
}

FileStream FileHandle::createFileStream(unsigned int mode) const
{
    // Check backend
    if (!m_backend)
    {
        return FileStream();
    }

    // Return stream
    return m_backend->createFileStream(mode);
}

eastl::string FileHandle::readFile() const
{
    // Check if file exists
    if (isFile())
    {
        // Open input stream
        FileStream stream = createFileStream(FileRead);
        if (!stream.isValid()) return "";

        // Return string
        return stream.read(stream.size());
    }

    // Error, not a valid file
    return "";
}

bool FileHandle::writeFile(const eastl::string & content)
{
    // Open output stream
    FileStream stream = createFileStream(FileWrite);
    if (!stream.isValid()) return false;

    // Write content to file
    stream.write(content.data(), content.size());

    // Done
    return true;
}

bool FileHandle::genericCopy(FileHandle & dest)
{
    // Check backend
    if (!m_backend)
    {
        return false;
    }

    // Open files
    FileStream in  = createFileStream(FileBinary);
    FileStream out = dest.createFileStream(FileBinary | FileWrite);

    if (!in.isValid() || !out.isValid())
    {
        // Error!
        return false;
    }

    // Copy file

    size_t srcSize = in.size();
    char* buffer = new char[srcSize];
    in.read(buffer, srcSize);
    out.write(buffer, srcSize);
    delete[] buffer;

    // Reload information on destination file
    dest.updateFileInfo();

    // Done
    return true;
}

bool FileHandle::genericMove(FileHandle & dest)
{
    // Copy file
    if (!genericCopy(dest))
    {
        return false;
    }

    // Remove source file
    return remove();
}


} // name cppfs
