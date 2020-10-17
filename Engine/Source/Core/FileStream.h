#pragma once

#include <EASTL/string.h>

struct SDL_RWops;

enum FileStreamMode
{
    FileAppend   = 0x01,  ///< A file stream that will append rather than write
    FileRead     = 0x02,  ///< A file stream that can read
    FileWrite    = 0x04,  ///< A file stream that can write
    FileBinary   = 0x08,  ///< File stream will be treated as binary
};

enum SeekFrom
{
    SeekStart,
    SeekCurr,
    SeekEnd
};

class FileStream
{
public:

    FileStream();

    FileStream(const FileStream & fileStream);

    FileStream(FileStream && fileStream);

    FileStream(eastl::string _path, unsigned int mode);

    ~FileStream();

    void Close();

    eastl::string Read(size_t length);

    void Read(char* buffer, size_t length);

    void Write(const char* buffer, size_t length);

    size_t Seek(size_t offset, SeekFrom from = SeekCurr);

    size_t Tell();

    size_t Size();

    bool IsValid();

private:
    SDL_RWops* rwops{ nullptr };
    eastl::string path{ "" };
    eastl::string modeString{ "" };
};