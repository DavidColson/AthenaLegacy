#pragma once

#include "AssetDatabase.h"

#include <SDL_audio.h>

struct Sound : Asset
{
    virtual void Load(Path path, AssetHandle handleForThis) override;
    ~Sound();

    SDL_AudioSpec spec;
    uint32_t length{ 0 };
    uint8_t* buffer{ nullptr };

    // @TODO: protect this struct from hot reloads
};
