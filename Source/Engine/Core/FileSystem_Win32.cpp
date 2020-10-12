#include "FileSystem.h"

#include "Log.h"
#include "FileStream.h"

#include <windows.h>

bool FileSys::Exists(const Path& path)
{
    if (path.IsEmpty())
        return false;

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

bool FileSys::IsEmpty(const Path& path)
{
    if (!Exists(path))
        return true;

    if (IsFile(path))
    {
        return FileSize(path) == 0;
    }
    else
    {
        Path wildCard = path / "*";
        WIN32_FIND_DATAA data;
        HANDLE handle = FindFirstFileA(wildCard.AsRawString(), &data);

        bool bSearch = true;
        while (bSearch)
        {
            if (FindNextFileA(handle, &data))
            {
                if (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, "..")) continue;
                return false;
            }
            else
            {
                if(GetLastError() == ERROR_NO_MORE_FILES)
                    bSearch = false;
            }
        }
        return true;
    }
}

bool FileSys::IsInUse(const Path& path)
{
    if (Exists(path) && IsFile(path))
    {
        FileStream stream = FileStream(path.AsString(), FileRead);
        if (!stream.IsValid()) return true;
    }
    return false;
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

eastl::vector<Path> FileSys::ListFiles(const Path& path)
{
    eastl::vector<Path> files;
    for (Path path : FileSys::DirectoryIterator(path))
    {
        if (path.HasFilename())
            files.push_back(path);
    }
    return files;
}

eastl::string FileSys::ReadWholeFile(Path path)
{
    // Check if file exists
    if (IsFile(path))
    {
        // Open input stream
        FileStream stream = FileStream(path.AsRawString(), FileRead);
        if (!stream.IsValid()) return "";

        // Return string
        return stream.Read(stream.Size());
    }

    // Error, not a valid file
    return "";
}

bool FileSys::WriteWholeFile(Path path, const eastl::string & content)
{
    // Open output stream
    FileStream stream = FileStream(path.AsRawString(), FileWrite);
    if (!stream.IsValid()) return false;

    // Write content to file
    stream.Write(content.data(), content.size());

    // Done
    return true;
}

Path FileSys::DirectoryIterator::Iterator::operator*() const
{
    return parentPath / currentPath;
}

bool FileSys::DirectoryIterator::Iterator::operator==(const Iterator& other) const
{
    return currentHandle == other.currentHandle;
}

bool FileSys::DirectoryIterator::Iterator::operator!=(const Iterator& other) const
{
    return currentHandle != other.currentHandle;
}

FileSys::DirectoryIterator::Iterator& FileSys::DirectoryIterator::Iterator::operator++()
{
    WIN32_FIND_DATAA data;
    if (!FindNextFileA((HANDLE)currentHandle, &data))
    {
        FindClose(currentHandle);
        currentHandle = (void*)INVALID_HANDLE_VALUE;
        currentPath = Path();
        return *this;
    }
    currentPath = Path(data.cFileName);
    return *this;
}

FileSys::DirectoryIterator::DirectoryIterator(Path directory)
{
    directoryToIterate = directory;;
}

const FileSys::DirectoryIterator::Iterator FileSys::DirectoryIterator::begin() const
{
    if (IsEmpty(directoryToIterate))
        return end();

    WIN32_FIND_DATAA data;
    HANDLE hFile = FindFirstFileA((directoryToIterate / "*").AsRawString(), &data);

    while (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, ".."))
        FindNextFileA(hFile, &data);

    return Iterator((void*)hFile, Path(data.cFileName), directoryToIterate);
}

const FileSys::DirectoryIterator::Iterator FileSys::DirectoryIterator::end() const
{
    return Iterator((void*)INVALID_HANDLE_VALUE, Path(), directoryToIterate);
}

Path FileSys::RecursiveDirectoryIterator::Iterator::operator*() const
{
    Path result = parentPath;
    for (const IteratorLevel& level : pathStack)
    {
        result /= level.path;
    }
    return result / currentPath;
}

bool FileSys::RecursiveDirectoryIterator::Iterator::operator==(const Iterator& other) const
{
    return currentHandle == other.currentHandle;
}

bool FileSys::RecursiveDirectoryIterator::Iterator::operator!=(const Iterator& other) const
{
    return currentHandle != other.currentHandle;
}

FileSys::RecursiveDirectoryIterator::Iterator& FileSys::RecursiveDirectoryIterator::Iterator::operator++()
{
    WIN32_FIND_DATAA data;

    // If current is a non-empty directory
    if (!skipNextIteration && IsDirectory(operator*()) && !IsEmpty(operator*()))
    {
        Path newPathToIterate = (operator*()) / "*";

        // Then add it to a stack of directories, and do find first file on it, then continue as normal until findNextFile fails.
        pathStack.push_back({currentPath, currentHandle});

        currentHandle = (void*)FindFirstFileA(newPathToIterate.AsRawString(), &data);
        while (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, ".."))
            FindNextFileA((HANDLE)currentHandle, &data);    

        currentPath = Path(data.cFileName);
        return *this;    
    }

    if (skipNextIteration)
        skipNextIteration = false;
    
    // Find the next file/folder
    if (!FindNextFileA((HANDLE)currentHandle, &data))
    {
        FindClose(currentHandle);

        // If the stack isn't empty, then get the last handle/data combo from the stack, popping it off, and carry on.
        if (!pathStack.empty())
        {
            currentHandle = pathStack.back().handle;
            currentPath = pathStack.back().path;
            pathStack.pop_back();

            // We've already searched the path at the top of the stack, no need to search it again on the next round so we skip it here
            skipNextIteration = true;
            return operator++();
        }
        // When stack is empty set currenthandle to invalid and finish
        currentHandle = (void*)INVALID_HANDLE_VALUE;
        currentPath = Path();
        return *this;
    }
    currentPath = Path(data.cFileName);
    return *this;
}

FileSys::RecursiveDirectoryIterator::RecursiveDirectoryIterator(Path directory)
{
    directoryToIterate = directory;
}

const FileSys::RecursiveDirectoryIterator::Iterator FileSys::RecursiveDirectoryIterator::begin() const
{
     if (IsEmpty(directoryToIterate))
        return end();

    WIN32_FIND_DATAA data;
    HANDLE hFile = FindFirstFileA((directoryToIterate / "*").AsRawString(), &data);

    while (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, ".."))
        FindNextFileA(hFile, &data);

    return Iterator((void*)hFile, Path(data.cFileName), directoryToIterate);
}

const FileSys::RecursiveDirectoryIterator::Iterator FileSys::RecursiveDirectoryIterator::end() const
{
    return Iterator((void*)INVALID_HANDLE_VALUE, Path(), directoryToIterate);
}