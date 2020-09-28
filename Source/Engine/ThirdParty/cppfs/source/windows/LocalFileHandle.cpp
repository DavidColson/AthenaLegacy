
#include <windows/LocalFileHandle.h>

#include <Log.h>

#include <fstream>

#include <windows.h>

#include <cppfs.h>
#include <FilePath.h>
#include <windows/LocalFileSystem.h>
#include <windows/LocalFileIterator.h>


namespace FileSys
{


LocalFileHandle::LocalFileHandle(eastl::shared_ptr<LocalFileSystem> fs, const eastl::string & path)
: m_fs(fs)
, m_path(path)
, m_fileInfo(nullptr)
{
}

LocalFileHandle::~LocalFileHandle()
{
    if (m_fileInfo)
    {
        delete (WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo;
    }
}

eastl::unique_ptr<AbstractFileHandleBackend> LocalFileHandle::clone() const
{
    return eastl::unique_ptr<AbstractFileHandleBackend>(new LocalFileHandle(m_fs, m_path));
}

AbstractFileSystem * LocalFileHandle::fs() const
{
    return static_cast<AbstractFileSystem *>(m_fs.get());
}

void LocalFileHandle::updateFileInfo()
{
    // Reset file information
    if (m_fileInfo)
    {
        delete (WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo;
        m_fileInfo = nullptr;
    }
}

eastl::string LocalFileHandle::path() const
{
    return m_path;
}

bool LocalFileHandle::exists() const
{
    readFileInfo();

    return (m_fileInfo != nullptr);
}

bool LocalFileHandle::isFile() const
{
    readFileInfo();

    if (m_fileInfo)
    {
        return (((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    return false;
}

bool LocalFileHandle::isDirectory() const
{
    readFileInfo();

    if (m_fileInfo)
    {
        return (((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    return false;
}

bool LocalFileHandle::isSymbolicLink() const
{
    readFileInfo();

    if (m_fileInfo)
    {
        return (((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }

    return false;
}

eastl::vector<eastl::string> LocalFileHandle::listFiles() const
{
    eastl::vector<eastl::string> entries;

    // Open directory
    WIN32_FIND_DATA findData;
    eastl::string query = FilePath(m_path).fullPath() + "/*";
    HANDLE findHandle = FindFirstFileA(query.c_str(), &findData);

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        return entries;
    }

    // Read directory entries
    do
    {
        // Get name
        eastl::string name = findData.cFileName;

        // Ignore . and ..
        if (name != ".." && name != ".")
        {
            entries.push_back(name);
        }
    } while (FindNextFile(findHandle, &findData));

    // Close directory
    FindClose(findHandle);

    // Return directory entries
    return entries;
}

eastl::unique_ptr<AbstractFileIteratorBackend> LocalFileHandle::begin() const
{
    return eastl::unique_ptr<AbstractFileIteratorBackend>(new LocalFileIterator(m_fs, m_path));
}

unsigned int LocalFileHandle::size() const
{
    readFileInfo();

    if (m_fileInfo)
    {
        // [TODO] Use 64bit numbers
        auto fileSizeH  = ((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->nFileSizeHigh;
        auto fileSizeL = ((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->nFileSizeLow;
        return static_cast<unsigned int>(static_cast<__int64>(fileSizeH) << 32 | fileSizeL);
    }

    return 0;
}

unsigned int LocalFileHandle::accessTime() const
{
    readFileInfo();

    if (m_fileInfo)
    {
        // [TODO] Use 64bit numbers
        FILETIME time = ((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->ftLastAccessTime;
        return static_cast<unsigned int>(static_cast<__int64>(time.dwHighDateTime) << 32 | time.dwLowDateTime);
    }

    return 0;
}

uint64_t LocalFileHandle::modificationTime() const
{
    readFileInfo();

    if (m_fileInfo)
    {
        // [TODO] Use 64bit numbers
        FILETIME time = ((WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo)->ftLastWriteTime;
        return static_cast<uint64_t>(time.dwHighDateTime) << 32 | static_cast<uint64_t>(time.dwLowDateTime);
    }

    return 0;
}

unsigned int LocalFileHandle::userId() const
{
    return 0;
}

void LocalFileHandle::setUserId(unsigned int)
{
}

unsigned int LocalFileHandle::groupId() const
{
    return 0;
}

void LocalFileHandle::setGroupId(unsigned int)
{
}

unsigned long LocalFileHandle::permissions() const
{
    return 0ul;
}

void LocalFileHandle::setPermissions(unsigned long)
{
}

bool LocalFileHandle::createDirectory()
{
    // Check directory
    if (exists()) return false;

    // Create directory
    if (!CreateDirectoryA(m_path.c_str(), nullptr))
    {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS)
            Log::Crit("Could not create directory because it already exists");
        if (error == ERROR_PATH_NOT_FOUND)
            Log::Crit("Could not create directory because one or more intermediate directories do not exist");
        return false;
    }

    // Done
    updateFileInfo();
    return true;
}

bool LocalFileHandle::removeDirectory()
{
    // Check directory
    if (!isDirectory()) return false;

    // Remove directory
    if (!RemoveDirectoryA(m_path.c_str()))
    {
        return false;
    }

    // Done
    updateFileInfo();
    return true;
}

bool LocalFileHandle::copy(AbstractFileHandleBackend & dest)
{
    // Check source file
    if (!isFile()) return false;

    // Get source and target filenames
    eastl::string src = m_path;
    eastl::string dst = dest.path();

    if (dest.isDirectory())
    {
        eastl::string filename = FilePath(m_path).fileName();
        dst = FilePath(dest.path()).resolve(filename).fullPath();
    }

    // Copy file
    if (!CopyFileA(src.c_str(), dst.c_str(), FALSE))
    {
        // Error!
        return false;
    }

    // Done
    updateFileInfo();
    return true;
}

bool LocalFileHandle::move(AbstractFileHandleBackend & dest)
{
    // Check source file
    if (!exists()) return false;

    // Get source and target filenames
    eastl::string src = m_path;
    eastl::string dst = dest.path();

    if (dest.isDirectory())
    {
        eastl::string filename = FilePath(m_path).fileName();
        dst = FilePath(dest.path()).resolve(filename).fullPath();
    }

    // Move file
    if (!MoveFileA(src.c_str(), dst.c_str()))
    {
        // Error!
        return false;
    }

    // Update path
    m_path = dst;
    updateFileInfo();

    // Done
    return true;
}

bool LocalFileHandle::createLink(AbstractFileHandleBackend & dest)
{
    // Check source file
    if (!isFile()) return false;

    // Get source and target filenames
    eastl::string src = m_path;
    eastl::string dst = dest.path();

    if (dest.isDirectory())
    {
        eastl::string filename = FilePath(m_path).fileName();
        dst = FilePath(dest.path()).resolve(filename).fullPath();
    }

    // Copy file
    if (!CreateHardLinkA(dst.c_str(), src.c_str(), 0))
    {
        // Error!
        return false;
    }

    // Done
    return true;
}

bool LocalFileHandle::createSymbolicLink(AbstractFileHandleBackend & dest)
{
    // Check source file
    if (!isFile()) return false;

    // Get source and target filenames
    eastl::string src = m_path;
    eastl::string dst = dest.path();

    if (dest.isDirectory())
    {
        eastl::string filename = FilePath(m_path).fileName();
        dst = FilePath(dest.path()).resolve(filename).fullPath();
    }

    // Copy file
    if (!CreateSymbolicLinkA(dst.c_str(), src.c_str(), 0))
    {
        // Error!
        return false;
    }

    // Done
    return true;
}

bool LocalFileHandle::rename(const eastl::string & filename)
{
    // Check file
    if (!exists()) return false;

    // Compose new file path
    eastl::string path = FilePath(FilePath(m_path).directoryPath()).resolve(filename).fullPath();

    // Rename
    if (!MoveFileA(m_path.c_str(), path.c_str()))
    {
        // Error!
        return false;
    }

    // Update path
    m_path = path;
    updateFileInfo();

    // Done
    return true;
}

bool LocalFileHandle::remove()
{
    // Check source file
    if (!isFile()) return false;

    // Delete file
    if (!DeleteFileA(m_path.c_str()))
    {
        return false;
    }

    // Done
    updateFileInfo();
    return true;
}

FileStream LocalFileHandle::createFileStream(unsigned int mode) const
{
    return FileStream(m_path.c_str(), mode);
}

void LocalFileHandle::readFileInfo() const
{
    // Check if file info has already been read
    if (m_fileInfo) return;

    // Create file information structure
    m_fileInfo = (void *)new WIN32_FILE_ATTRIBUTE_DATA;

    // Get file info
    if (!GetFileAttributesExA(m_path.c_str(), GetFileExInfoStandard, (WIN32_FILE_ATTRIBUTE_DATA*)m_fileInfo))
    {
        // Error!
        delete (WIN32_FILE_ATTRIBUTE_DATA *)m_fileInfo;
        m_fileInfo = nullptr;
    }
}


} // namespace FileSys
