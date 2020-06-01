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
    };
    eastl::map<uint64_t, AssetMeta> assetMetas;
    //eastl::vector<HotReloadAsset> hotReloadWatchers;
}

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
}
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
    assets[AssetHandle(identifier).id] = pAsset;
}