#pragma once

#include "AssetDatabase.h"

struct Text : Asset
{
    virtual void Load(Path path, AssetHandle handleForThis) override;

    eastl::string contents;
};
