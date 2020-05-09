#include "AssetDatabase.h"

#include <EASTL/map.h>
#include <EASTL/string.h>

namespace
{
    eastl::map<uint64_t, Asset*> assets;
    eastl::map<uint64_t, eastl::string> assetIdentifiers;
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


AssetHandle:: AssetHandle(eastl::string identifier, AssetType _type)
{
    id = CalculateFNV(identifier.c_str());
    type = _type;
    AssetDB::RegisterAssetIdentifier(identifier, id);
}

Asset* AssetDB::GetAssetData(AssetHandle handle)
{
    Asset* pAsset;
    if (assets.count(handle.id) == 0)
    {
        switch (handle.type)
        {
        case AssetType::Text:
            pAsset = new Text;
            pAsset->Load(assetIdentifiers[handle.id]);
            break;
        default:
            break;
        }
    }
    else
    {
        pAsset = assets[handle.id];
    }
    return pAsset;
}

void AssetDB::FreeAsset(AssetHandle handle)
{
    if (assets.count(handle.id) == 0)
        return;

    Asset* pAsset = assets[handle.id];
    delete pAsset;
    assets.erase(handle.id);
}

void AssetDB::RegisterAssetIdentifier(eastl::string identifier, uint64_t id)
{
    assetIdentifiers[id] = identifier;
}