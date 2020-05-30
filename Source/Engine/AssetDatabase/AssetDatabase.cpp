#include "AssetDatabase.h"

#include "Mesh.h"
#include "Log.h"
#include "Model.h"
#include "Text.h"
#include "Sound.h"

#include <EASTL/map.h>
#include <EASTL/string.h>

#include <filesystem>

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

AssetHandle::AssetHandle(eastl::string identifier)
{
    id = CalculateFNV(identifier.c_str());
    assetIdentifiers[id] = identifier;
}

eastl::string ExtractFileExtension(eastl::string path)
{   
    int extensionStart = (int)path.length();
    int extensionEnd = 0;
    for (int i = (int)path.length() - 1; i >= 0; i--)
    {
        if (path[i] == ':')
            extensionStart = i;
        
        if (path[i] == '.')
            extensionEnd = i + 1;
    }
    return path.substr(extensionEnd, extensionStart - extensionEnd);
}

Asset* AssetDB::GetAssetRaw(AssetHandle handle)
{
    Asset* pAsset;
    if (assets.count(handle.id) == 0)
    {
        eastl::string identifier = assetIdentifiers[handle.id];
        eastl::string fileType = ExtractFileExtension(identifier);

        // Composite asset
        eastl::string topLevelAssetIdentifier;
        if (IsSubasset(handle, topLevelAssetIdentifier))
        {
            AssetHandle topAssetHandle(topLevelAssetIdentifier);

            Asset* pTopAsset; 
            if (fileType == "gltf")
            {
                pTopAsset = new Model();
            }
            pTopAsset->Load(topLevelAssetIdentifier);
            pAsset = assets[handle.id];
        }
        // Non composite asset
        else
        {
            if (fileType == "txt")
            {
                pAsset = new Text();
            }
            else if (fileType == "wav")
            {
                pAsset = new Sound();
            }
            else
            {
                Log::Crit("Attempting to load an unsupported asset type. Will crash imminently");
            }
            
            pAsset->Load(assetIdentifiers[handle.id]);
        }
    }
    else
    {
        pAsset = assets[handle.id];
    }
    return pAsset;
}

eastl::string AssetDB::GetAssetIdentifier(AssetHandle handle)
{
    if (assetIdentifiers.count(handle.id) == 0)
        return "";

    return assetIdentifiers[handle.id];
}

void AssetDB::FreeAsset(AssetHandle handle)
{
    if (assets.count(handle.id) == 0)
        return;

    Asset* pAsset = assets[handle.id];
    delete pAsset;
    assets.erase(handle.id);
}

bool AssetDB::IsSubasset(AssetHandle handle, eastl::string& outTopLevelIdentifier)
{
    eastl::string identifier = assetIdentifiers[handle.id];

    bool foundSubAssetIdentifier = false;
    int location = (int)identifier.length() - 1;
    for (int i = (int)identifier.length() - 1; i >= 0; i--)
    {
        if (identifier[i] == ':')
        {
            foundSubAssetIdentifier = true;
            break;
        }
        location--;
    }
    outTopLevelIdentifier = identifier.substr(0, location);
    return foundSubAssetIdentifier;
}

void AssetDB::RegisterAsset(Asset* pAsset, eastl::string identifier)
{
    assets[AssetHandle(identifier).id] = pAsset;
}