#pragma once

#include <EASTL/string.h>
#include <Path.h>

enum class AssetType
{
    Text,
    Mesh,
    Model,
    Sound,
    Shader,
    Font
};

struct AssetHandle
{
    AssetHandle() : id(0) {}
    AssetHandle(eastl::string identifier);
    AssetHandle(uint64_t id);

    AssetHandle(const AssetHandle& copy);
	AssetHandle(AssetHandle&& move);
	AssetHandle& operator=(const AssetHandle& copy);
	AssetHandle& operator=(AssetHandle&& move);

    ~AssetHandle();

    uint64_t id{ 0 };
};

struct Asset
{
    virtual void Load(Path loadPath, AssetHandle handleForThis) = 0;
    virtual void Reload(Path loadPath, AssetHandle handleForThis) {};
    virtual ~Asset() {};

    bool bOverrideReload{ false };
};

// Some notes:
// I think editor behaviour of asset database should behave differently. Rather than on demand loading assets as they're required
// It should just scan the Assets directory and have "tracked assets", which are in the database, but not actually loaded until they're really required. 
// If we're just showing the user some stuff that they can use in their level, we may not want to load everything

namespace AssetDB
{
    template<typename T>
    T* GetAsset(AssetHandle handle)
    {
        return static_cast<T*>(GetAssetRaw(handle));
    }

    Asset* GetAssetRaw(AssetHandle handle);

    eastl::string GetAssetIdentifier(AssetHandle handle);

    void FreeAsset(AssetHandle handle);
    
    bool IsSubasset(AssetHandle handle);

    void RegisterAsset(Asset* pAsset, eastl::string identifier);

    void UpdateHotReloading();

    void CollectGarbage();
}