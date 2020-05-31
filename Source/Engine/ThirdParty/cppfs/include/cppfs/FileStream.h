#pragma once

#include <EASTL/string.h>

struct SDL_RWops;

namespace FileSys
{

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
    FileStream(eastl::string path, unsigned int mode);
    ~FileStream();

    void close();

    eastl::string read(size_t length);
    
    void read(char* buffer, size_t length);

    void write(const char* buffer, size_t length);

    size_t seek(size_t offset, SeekFrom from = SeekCurr);

    size_t tell();

    size_t size();

    bool isValid();

private:
    SDL_RWops* rwops{ nullptr };
};

}