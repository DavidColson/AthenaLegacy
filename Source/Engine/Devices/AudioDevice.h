#pragma once

#include <SDL.h>
#include <EASTL/shared_ptr.h>

#include "Log.h"

typedef unsigned long long SoundID;
typedef unsigned long long LoadedSoundHandle;

struct LoadedSound
{
    uint8_t* buffer{ nullptr };
    uint32_t length{ 0 };
    SDL_AudioSpec spec;

    ~LoadedSound()
    {
        SDL_FreeWAV(buffer);
    }
};

typedef eastl::shared_ptr<LoadedSound> LoadedSoundPtr;
typedef eastl::weak_ptr<LoadedSound> LoadedSoundWeakPtr;

namespace AudioDevice
{
    void Initialize();

    LoadedSoundPtr LoadSound(const char* fileName);

    SoundID PlaySound(LoadedSoundPtr sound, float volume, bool loop);

    void StopSound(SoundID sound);

    void PauseSound(SoundID sound);

    void UnPauseSound(SoundID sound);

    void Destroy();
}