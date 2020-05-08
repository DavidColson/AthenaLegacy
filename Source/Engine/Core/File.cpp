#include "File.h"

#include "Log.h"

#include <SDL_rwops.h>

eastl::string File::ReadWholeFile(eastl::string filepath, bool binary)
{
    const char* mode = binary ? "rb" : "r";
    SDL_RWops* rw = SDL_RWFromFile(filepath.c_str(), mode);
	if (rw == nullptr)
	{
		Log::Warn("File %s failed to load", filepath.c_str());
		return eastl::string();
	}

	size_t fileSize = SDL_RWsize(rw);
	char* buffer = new char[fileSize];

	SDL_RWread(rw, buffer, sizeof(char) * fileSize, 1);
	SDL_RWclose(rw);
    
    eastl::string result = buffer;
    delete[] buffer;

	return result;
}

void File::WriteWholeFile(eastl::string filepath, eastl::string contents, bool binary)
{
    const char* mode = binary ? "wb" : "w";
    SDL_RWops* rw = SDL_RWFromFile(filepath.c_str(), mode);
	if (rw == nullptr)
	{
		Log::Warn("File %s failed to load", filepath);
        return;
	}

    if (SDL_RWwrite(rw, contents.c_str(), 1, contents.size()) != contents.size()) 
    {
        Log::Warn("Couldn't fully write to file %s", filepath.c_str());
    }
    SDL_RWclose(rw);
}