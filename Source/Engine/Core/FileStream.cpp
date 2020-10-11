#include <FileStream.h>

#include <SDL_rwops.h>
#include <Log.h>


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

    rwops = SDL_RWFromFile(_path.c_str(), modeString.c_str());
    if (rwops == nullptr)
    {
        Log::Crit("Opening file stream for file %s failed with error: %s", _path.c_str(), SDL_GetError());
    }

    path = _path;
}

FileStream::~FileStream()
{
    Close();
}

void FileStream::Close()
{
    if (rwops == nullptr)
        return;
    
    SDL_RWclose(rwops);
    rwops = nullptr;
}

eastl::string FileStream::Read(size_t length)
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

void FileStream::Read(char* buffer, size_t length)
{
    if (rwops == nullptr)
        return;

    SDL_RWread(rwops, buffer, length, 1);
}

void FileStream::Write(const char* buffer, size_t length)
{
    if (rwops == nullptr)
        return;

    SDL_RWwrite(rwops, buffer, length, 1);
}

size_t FileStream::Seek(size_t offset, SeekFrom from)
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

size_t FileStream::Tell()
{
    if (rwops == nullptr)
        return 0;

    return SDL_RWtell(rwops);
}

size_t FileStream::Size()
{
    if (rwops == nullptr)
        return 0;

    return SDL_RWsize(rwops);
}

bool FileStream::IsValid()
{
    return rwops != nullptr;
}