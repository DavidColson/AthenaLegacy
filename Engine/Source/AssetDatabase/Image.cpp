#include "Image.h"

#include "FileSystem.h"
#include "Log.h"
#include "GraphicsDevice.h"

#include <stb/stb_image.h>

void Image::Load(Path path, AssetHandle handleForThis)
{
    imgData = stbi_load(path.AsRawString(), &xSize, &ySize, &nChannels, 0);

    if (imgData == nullptr)
    {
        Log::Crit("Image load failed on file %s for reason %s", path.AsRawString(), stbi_failure_reason());
    }

    gpuHandle = GfxDevice::CreateTexture(xSize, ySize, TextureFormat::RGBA8, imgData, path.AsString());
}

Image::~Image()
{
    stbi_image_free(imgData);
}