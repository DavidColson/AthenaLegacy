#pragma once

#include "AssetDatabase.h"

struct Text : Asset
{
    virtual void Load(Path path) override;

    eastl::string contents;
};
