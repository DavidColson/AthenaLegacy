#pragma once

#include "File.h"

#include <EASTL/string.h>

struct Asset
{
    virtual void Load(eastl::string path) = 0;
};

struct Text : Asset
{
    virtual void Load(eastl::string path) override
    {
        contents = File::ReadWholeFile(path);
    }

    eastl::string contents;
};

enum class AssetType
{
    Text,
    Mesh,
    Sound,
    Shader,
    Font
};

struct AssetHandle
{
    AssetHandle() : id(0) {}
    AssetHandle(eastl::string identifier, AssetType _type);

    uint64_t id;
    AssetType type;
};

namespace AssetDB
{
    Asset* GetAssetData(AssetHandle handle);

    void FreeAsset(AssetHandle handle);

    void RegisterAssetIdentifier(eastl::string identifier, uint64_t id);
}