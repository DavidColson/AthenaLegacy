#pragma once

#include "AssetDatabase.h"

struct Text : Asset
{
    virtual void Load(FileSys::FilePath path) override;

    eastl::string contents;
};
