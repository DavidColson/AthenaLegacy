#pragma once

#include "Path.h"

namespace FileSys
{
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