#include "AssetDatabase.h"

#include "Mesh.h"
#include "Log.h"
#include "Model.h"
#include "Text.h"
#include "Sound.h"
#include "Shader.h"
#include "Image.h"
#include "Font.h"
#include "FileSystem.h"

#include <EASTL/map.h>
#include <EASTL/string.h>

namespace
{
    eastl::map<uint64_t, Asset*> assets;

    struct AssetMeta
    {
        eastl::string fullIdentifier;
        eastl::string subAssetName;
        Path path;
        Path realPath;
        uint32_t refCount{ 0 };
    };
    eastl::map<uint64_t, AssetMeta> assetMetas;

    struct HotReloadingAsset
    {
        uint64_t assetID;
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
        meta.path = Path(identifier.substr(0, subAssetPos));
        meta.subAssetName = subAssetName;
        meta.fullIdentifier = identifier;
        assetMetas[id] = meta;
    }
    assetMetas[id].refCount += 1;
}

AssetHandle::AssetHandle(uint64_t id)
{
    if (assetMetas.count(id) >= 0)
    {
        assetMetas[id].refCount += 1;
    }
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

bool AssetHandle::operator==(const AssetHandle& other)
{
    return id == other.id;
}

bool AssetHandle::operator!=(const AssetHandle& other)
{
    return id != other.id;
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
    // TODO: consider the asset type when someone gets an asset, if they request an asset that exists, but it's a different type, gracefully fail
    // TODO: Test if the path actually points to a real file, and error if not
    if (assets.count(handle.id) == 0)
    {
        if (assetMetas[handle.id].path.IsEmpty())
            return nullptr;

        Asset* pAsset;
        Path& path = assetMetas[handle.id].path;
        eastl::string fileType = path.Extension().AsString();

        if (fileType == ".txt")
        {
            pAsset = new Text();
        }
        else if (fileType == ".png")
        {
            pAsset = new Image();
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
            Log::Crit("Attempting to load an unsupported asset type");
            return nullptr;
        }
        
        // If we're attempting to load a subasset, we're actually calling load on the parent asset, 
        // so correct the handle to remove the subasset from the identifier
        AssetHandle handleForThis = handle;
        if (IsSubasset(handle))
            handleForThis = AssetHandle(assetMetas[handle.id].path.AsString());

        Path fileInGameResources = Path(Engine::GetConfig().gameResourcesPath) / path;
        Path fileInEngineResources = Path(Engine::GetConfig().engineResourcesPath) / path;
        if (FileSys::Exists(fileInGameResources))
        {
            assetMetas[handle.id].realPath = fileInGameResources;
            pAsset->Load(fileInGameResources, handleForThis);
        }
        else if (FileSys::Exists(fileInEngineResources))
        {
            assetMetas[handle.id].realPath = fileInEngineResources;
            pAsset->Load(fileInEngineResources, handleForThis);
        }
        else
        {
            Log::Crit("Unable to load asset %s from engine (%s) or game (%s) resource folders", path.AsRawString(), Engine::GetConfig().engineResourcesPath.c_str(), Engine::GetConfig().gameResourcesPath.c_str());
            return nullptr;
        }

        RegisterAsset(pAsset, assetMetas[handleForThis.id].fullIdentifier);
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
    assets.erase(handle.id);
    
    // Don't actually free subassets, let their parents be freed instead
    // TODO: Make subassets force their parents to have a ref count so they don't get deleted
    if (!assetMetas[handle.id].subAssetName.empty()) 
        return;

    delete pAsset;
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
    if (Engine::GetConfig().hotReloadingAssetsEnabled)
    {
        for (const HotReloadingAsset& hot : hotReloadWatches)
        {
            if (hot.assetID == handle.id)
                return;
        }
        // Don't hot reload audio
        // @Improvement actually define an asset type enum or something, this is icky
        if (assetMetas[handle.id].path.Extension() != ".wav")
        {
            HotReloadingAsset hot;
            hot.assetID = handle.id; 
            hot.cacheLastModificationTime = FileSys::LastWriteTime(assetMetas[hot.assetID].realPath);
            hotReloadWatches.push_back(hot);
        }
    }
}

void AssetDB::UpdateHotReloading()
{
    for (HotReloadingAsset& hot : hotReloadWatches)
    {
        if (hot.cacheLastModificationTime != FileSys::LastWriteTime(assetMetas[hot.assetID].realPath))
        {
            if (assetMetas[hot.assetID].refCount == 0)
                continue; // Don't hot reload an asset no one is referencing

            if (FileSys::IsInUse(assetMetas[hot.assetID].realPath))
                continue;

            Asset* pAsset = assets[hot.assetID];
            if (pAsset->bOverrideReload)
            {
                pAsset->Reload(assetMetas[hot.assetID].realPath, AssetHandle(hot.assetID));
            }
            else
            {
                assets.erase(hot.assetID);
                delete pAsset;

                pAsset = GetAssetRaw(AssetHandle(hot.assetID));
            }

            hot.cacheLastModificationTime = FileSys::LastWriteTime(assetMetas[hot.assetID].realPath);
            Log::Info("Reloaded asset %s..", assetMetas[hot.assetID].realPath.AsRawString());
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