#pragma once

#include "Mesh.h"

struct Model : Asset
{
    eastl::vector<Mesh> meshes;

    virtual void Load(FileSys::FilePath path) override;
};