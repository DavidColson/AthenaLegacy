#pragma once

#include "Mesh.h"

struct Model : Asset
{
    eastl::vector<Mesh> meshes;
    eastl::vector<AssetHandle> subAssets;

    virtual void Load(Path path) override;
    ~Model() {}
};