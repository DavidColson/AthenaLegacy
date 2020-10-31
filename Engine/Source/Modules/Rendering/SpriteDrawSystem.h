#pragma once

#include "GraphicsDevice.h"
#include "AssetDatabase.h"

struct Scene;
struct Mesh;
struct FrameContext;

struct CSprite
{
    AssetHandle spriteHandle;

    REFLECT()
};

namespace SpriteDrawSystem
{	
    void Initialize();
	void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
	void Destroy();
}