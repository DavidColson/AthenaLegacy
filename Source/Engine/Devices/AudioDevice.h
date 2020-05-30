#pragma once

#include <EASTL/shared_ptr.h>

#include "Log.h"
#include "AssetDatabase.h"

#include <SDL_audio.h>

typedef unsigned long long SoundID;

namespace AudioDevice
{
    void Initialize();

    SoundID PlaySound(AssetHandle soundAsset, float volume, bool loop);

    void StopSound(SoundID sound);

    void PauseSound(SoundID sound);

    void UnPauseSound(SoundID sound);

    void Destroy();
}