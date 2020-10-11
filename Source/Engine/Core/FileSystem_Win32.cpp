#include "FileSystem.h"

#include "Log.h"

#include <windows.h>

bool FileSys::Exists(const Path& path)
{
    WIN32_FIND_DATAA findFileData;
    HANDLE handle = FindFirstFileA(path.RemoveTrailingSlash().AsRawString(), &findFileData);
    
    if (handle == INVALID_HANDLE_VALUE)
        return false;
    return true;
}

bool FileSys::IsDirectory(const Path& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path.AsRawString(), GetFileExInfoStandard, &fileInfo))
    {
        // Give Error
        return 0;
    }

    return (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool FileSys::IsFile(const Path& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path.AsRawString(), GetFileExInfoStandard, &fileInfo))
    {
        // Give Error
        return 0;
    }

    return (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

uint64_t FileSys::LastWriteTime(const Path& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path.AsRawString(), GetFileExInfoStandard, &fileInfo))
    {
        // Give Error
        return 0;
    }

    FILETIME time = fileInfo.ftLastWriteTime;
    return static_cast<uint64_t>(time.dwHighDateTime) << 32 | static_cast<uint64_t>(time.dwLowDateTime);
}

uint64_t FileSys::LastAccessTime(const Path& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path.AsRawString(), GetFileExInfoStandard, &fileInfo))
    {
        // Give Error
        return 0;
    }

    FILETIME time = fileInfo.ftLastAccessTime;
    return static_cast<uint64_t>(time.dwHighDateTime) << 32 | static_cast<uint64_t>(time.dwLowDateTime);
}

uint64_t FileSys::FileSize(const Path& path)
{
    if (!IsFile(path))
        return 0;

    // TODO: Is file check
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path.AsRawString(), GetFileExInfoStandard, &fileInfo))
    {
        // Give Error
        return 0;
    }
    return static_cast<uint64_t>(fileInfo.nFileSizeHigh) << 32 | static_cast<uint64_t>(fileInfo.nFileSizeLow);
}

Path FileSys::CurrentDirectory()
{
    char fileName[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, fileName);
    return Path(fileName);
}

bool FileSys::Move(const Path& existingPath, const Path& newPath, bool createNecessaryDirs)
{
    if (createNecessaryDirs)
    {
        NewDirectories(newPath.ParentPath());
    }
    
    if (Exists(existingPath))
    {
        bool result = MoveFileA(existingPath.AsRawString(), newPath.AsRawString());

        if (!result)
            return false; // Give Error
        
        return true;
    }
    return false; // Give error
}

bool FileSys::NewDirectory(const Path& newPath)
{
    bool result = CreateDirectoryA(newPath.AsRawString(), NULL);
    // Give appropriate errors in response
    return result;
}

bool FileSys::NewDirectories(const Path& newPath)
{
    Path cumulator = newPath.RootPath();
    for (Path path : newPath.RelativePath())  
    {
        cumulator /= path;
        if (!Exists(path))
        {
            if (NewDirectory(cumulator) == false)
                return false;
        }
    } 
    return true;
}