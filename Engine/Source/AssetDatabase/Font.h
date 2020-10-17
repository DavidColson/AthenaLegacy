#pragma once

#include "AssetDatabase.h"
#include "GraphicsDevice.h"
#include "Rendering/FontSystem.h"

#include <EASTL/vector.h>

struct Font : Asset
{
    virtual void Load(Path path) override;
    ~Font();

	TextureHandle fontTexture;
	eastl::vector<Character> characters;
};
