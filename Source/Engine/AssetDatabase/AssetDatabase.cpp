#include "AssetDatabase.h"

#include "Mesh.h"
#include "Log.h"
#include "Model.h"
#include "Text.h"
#include "Sound.h"
#include "Shader.h"
#include "Font.h"

#include <EASTL/map.h>
#include <EASTL/string.h>

namespace
{
    eastl::map<uint64_t, Asset*> assets;

    struct AssetMeta
    {
        eastl::string fullIdentifier;
        eastl::string subAssetName;
        FileSys::FilePath path;
        uint32_t refCount{ 0 };
    };
    eastl::map<uint64_t, AssetMeta> assetMetas;

    struct HotReloadingAsset
    {
        AssetHandle asset;
        uint64_t cacheLastModificationTime;
    };
    eastl::vector<HotReloadingAsset> hotReloadWatches;
}


// Asset Handle
// ****************************

uint64_t CalculateFNV(const char* str)
{
	uint64_t offset = 14695981039346656037u;
	uint64_t prime = 1099511628211u;

	uint64_t hash = offset;
	while (*str != 0)
	{
		hash ^= *str++;
		hash *= prime;
	}
	return hash;
}

AssetHandle::AssetHandle(eastl::string identifier)
{
    id = CalculateFNV(identifier.c_str());
    if (assetMetas.count(id) == 0)
    {
        eastl::string subAssetName;
        size_t subAssetPos  = identifier.find_last_of(":");
        if (subAssetPos != eastl::string::npos)
            subAssetName = identifier.substr(subAssetPos + 1);

        AssetMeta meta;
        meta.path = FileSys::FilePath(identifier.substr(0, subAssetPos));
        meta.subAssetName = subAssetName;
        meta.fullIdentifier = identifier;
        assetMetas[id] = meta;
    }
    assetMetas[id].refCount += 1;
}

AssetHandle::AssetHandle(const AssetHandle& copy)
{
    id = copy.id;
    assetMetas[id].refCount += 1;
}

AssetHandle::AssetHandle(AssetHandle&& move)
{
    // Don't increment ref counter
    id = move.id;
    move.id = 0;
}

AssetHandle& AssetHandle::operator=(const AssetHandle& copy)
{
    id = copy.id;
    assetMetas[id].refCount += 1;
    return *this;
}

AssetHandle& AssetHandle::operator=(AssetHandle&& move)
{
    // Don't increment ref counter
    id = move.id;
    move.id = 0;
    return *this;
}

AssetHandle::~AssetHandle()
{
    if (id != 0)
        assetMetas[id].refCount -= 1;
}



// Asset Database
// ****************************

Asset* AssetDB::GetAssetRaw(AssetHandle handle)
{
    if (assets.count(handle.id) == 0)
    {
        Asset* pAsset;
        FileSys::FilePath& path = assetMetas[handle.id].path;
        eastl::string fileType = path.extension();

        if (fileType == ".txt")
        {
            pAsset = new Text();
        }
        else if (fileType == ".wav")
        {
            pAsset = new Sound();
        }
        else if (fileType == ".hlsl")
        {
            pAsset = new Shader();
        }
        else if (fileType == ".otf" || fileType == ".ttf")
        {
            pAsset = new Font();
        }
        else if (fileType == ".gltf")
        {
            pAsset = new Model();
        }
        else
        {
            Log::Crit("Attempting to load an unsupported asset type. Will crash imminently");
        }
        
        pAsset->Load(path);

        // If a subasset was requested, we've just loaded the parent asset, 
        // so register the parent with a different identifier pointing to just the file
        if (IsSubasset(handle))
            RegisterAsset(pAsset, assetMetas[handle.id].path.fullPath());
        else
            RegisterAsset(pAsset, assetMetas[handle.id].fullIdentifier);
    }

    return assets[handle.id];
}

eastl::string AssetDB::GetAssetIdentifier(AssetHandle handle)
{
    if (assetMetas.count(handle.id) == 0)
        return "";

    return assetMetas[handle.id].fullIdentifier;
}

void AssetDB::FreeAsset(AssetHandle handle)
{
    if (assets.count(handle.id) == 0)
        return;

    Asset* pAsset = assets[handle.id];
    delete pAsset;
    assets.erase(handle.id);
}

bool AssetDB::IsSubasset(AssetHandle handle)
{
    return !assetMetas[handle.id].subAssetName.empty();
}

void AssetDB::RegisterAsset(Asset* pAsset, eastl::string identifier)
{
    AssetHandle handle = AssetHandle(identifier); 
    assets[handle.id] = pAsset;

    // See if this asset is already in the hot reload watches, if so skip the rest of the function
    for (const HotReloadingAsset& hot : hotReloadWatches)
    {
        if (hot.asset.id == handle.id)
            return;
    }

    // Don't hot reload audio
    // @Improvement actually define an asset type enum or something, this is icky
    if (assetMetas[handle.id].path.extension() != ".wav")
    {
        HotReloadingAsset hot;
        hot.asset = handle;
        hot.cacheLastModificationTime = FileSys::open(assetMetas[hot.asset.id].path).modificationTime();
        hotReloadWatches.push_back(hot);
        assetMetas[handle.id].refCount -= 1; // Assets in the hot reloader should not count as real references
    }
}

void AssetDB::UpdateHotReloading()
{
    for (HotReloadingAsset& hot : hotReloadWatches)
    {
        FileSys::FileHandle file = FileSys::open(assetMetas[hot.asset.id].path);
        if (hot.cacheLastModificationTime != file.modificationTime())
        {
            if (assetMetas[hot.asset.id].refCount == 0)
                continue; // Don't hot reload an asset no one is referencing

            if (file.isInUse())
                continue;

            Asset* pAsset = assets[hot.asset.id];
            assets.erase(hot.asset.id);
            delete pAsset;

            pAsset = GetAssetRaw(hot.asset);
            hot.cacheLastModificationTime = file.modificationTime();
        }
    }
}

void AssetDB::CollectGarbage()
{
    for (const eastl::pair<uint64_t, AssetMeta>& assetMeta : assetMetas)
    {
        if (assetMeta.second.refCount == 0)
        {
            FreeAsset(AssetHandle(assetMeta.second.fullIdentifier));
        }
    }
}