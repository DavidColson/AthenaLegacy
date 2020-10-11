#pragma once

#include "Path.h"

namespace FileSys
{
    class DirectoryIterator
    {
    public:
        DirectoryIterator(Path directory);

        struct Iterator
        {
            Iterator(void* handle, Path path, Path parent) : currentHandle(handle), currentPath(path), parentPath(parent) {}

            Path operator*() const;
            bool operator==(const Iterator& other) const;
            bool operator!=(const Iterator& other) const;

            Iterator& operator++();
            
            Path parentPath;
            Path currentPath;
            void* currentHandle;
        };

        const Iterator begin() const;

        const Iterator end() const;
    private:
        Path directoryToIterate;
    };

    bool Exists(const Path& path);

    bool IsDirectory(const Path& path);

    bool IsFile(const Path& path);

    uint64_t LastWriteTime(const Path& path);

    uint64_t LastAccessTime(const Path& path);

    uint64_t FileSize(const Path& path);

    Path CurrentDirectory();

    bool Move(const Path& existingPath, const Path& newPath, bool createNecessaryDirs = false);

    bool NewDirectory(const Path& newPath);

    bool NewDirectories(const Path& newPath);
}