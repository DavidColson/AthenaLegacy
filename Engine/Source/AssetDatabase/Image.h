#pragma once

#include "AssetDatabase.h"
#include "GraphicsDevice.h"

struct Image : Asset
{
    virtual void Load(Path path, AssetHandle handleForThis) override;

    ~Image();

    int xSize;
    int ySize;
    int nChannels;
    unsigned char* imgData;
    TextureHandle gpuHandle;
};
