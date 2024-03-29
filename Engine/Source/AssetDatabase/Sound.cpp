#include "Sound.h"

#include "Log.h"

void Sound::Load(Path path, AssetHandle handleForThis)
{
    if (SDL_LoadWAV(path.AsRawString(), &spec, &buffer, &length) == nullptr)
    {
        Log::Warn("%s", SDL_GetError());
    }

    if (spec.channels == 1) // doubling our actual buffer length since we reuse the samples for stereo
        length *= 2;

    // @TODO Ensure the loaded audio file is of the correct format and give errors otherwise
}

Sound::~Sound()
{
    SDL_FreeWAV(buffer);
}