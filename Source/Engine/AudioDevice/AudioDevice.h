#pragma once

#include <SDL.h>

typedef unsigned long long SoundID;
typedef unsigned long long LoadedSoundHandle;

namespace AudioDevice
{
    void Initialize();

    LoadedSoundHandle LoadSound(const char* fileName);

    SoundID PlaySound(LoadedSoundHandle sound, float volume, bool loop);

    void PauseSound(SoundID sound);

    void UnPauseSound(SoundID sound);

    void Update();

    void Destroy();
}