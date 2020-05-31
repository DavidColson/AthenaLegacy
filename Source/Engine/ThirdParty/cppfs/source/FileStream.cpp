#include <cppfs/FileStream.h>

#include <SDL_rwops.h>

#include <cppfs/cppfs.h>

namespace FileSys
{

FileStream::FileStream()
{
}

FileStream::FileStream(const FileStream & fileStream)
{
    // Copy
    path = fileStream.path;
    modeString = fileStream.modeString;
    rwops = SDL_RWFromFile(path.c_str(), modeString.c_str());
}

FileStream::FileStream(FileStream && fileStream)
{
    // Move
    rwops = fileStream.rwops;
    path = fileStream.path;
    modeString = fileStream.modeString;

    fileStream.rwops = nullptr;
    fileStream.path = "";
    fileStream.modeString = "";
}

FileStream::FileStream(eastl::string _path, unsigned int mode)
{
    modeString = "";

    if (mode & FileWrite)
    {
        modeString += "w";
        if (mode & FileRead)
            modeString += "+";
    }
    else if (mode & FileAppend)
    {
        modeString += "a";
        if (mode & FileRead)
            modeString += "+";
    }
    else if (mode & FileRead)
    {
        modeString += "r";
    }

    if (mode & FileBinary)
    {
        modeString += "b";
    }

    rwops = SDL_RWFromFile(path.c_str(), modeString.c_str());
    path = _path;
}

FileStream::~FileStream()
{
    close();
}

void FileStream::close()
{
    if (rwops == nullptr)
        return;
    
    SDL_RWclose(rwops);
    rwops = nullptr;
}

eastl::string FileStream::read(size_t length)
{
    if (rwops == nullptr)
        return "";

	char* buffer = new char[length + 1];
	SDL_RWread(rwops, buffer, length, 1);
    buffer[length] = '\0';

    eastl::string result = buffer;
    delete[] buffer;
    return result;
}

void FileStream::read(char* buffer, size_t length)
{
    if (rwops == nullptr)
        return;

    SDL_RWread(rwops, buffer, length, 1);
}

void FileStream::write(const char* buffer, size_t length)
{
    if (rwops == nullptr)
        return;

    SDL_RWwrite(rwops, buffer, length, 1);
}

size_t FileStream::seek(size_t offset, SeekFrom from)
{
    if (rwops == nullptr)
        return 0;

    if (from == SeekStart)
        return SDL_RWseek(rwops, offset, RW_SEEK_SET);
    else if(from == SeekEnd)
        return SDL_RWseek(rwops, offset, RW_SEEK_END);
    else
        return SDL_RWseek(rwops, offset, RW_SEEK_CUR);
}

size_t FileStream::tell()
{
    if (rwops == nullptr)
        return 0;

    return SDL_RWtell(rwops);
}

size_t FileStream::size()
{
    if (rwops == nullptr)
        return 0;

    return SDL_RWsize(rwops);
}

bool FileStream::isValid()
{
    return rwops != nullptr;
}

}