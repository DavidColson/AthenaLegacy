#pragma once

#include "AssetDatabase.h"

struct Text : Asset
{
    virtual void Load(eastl::string path) override;

    eastl::string contents;
};
