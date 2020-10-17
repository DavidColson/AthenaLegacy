#pragma once

#include "GraphicsDevice.h"
#include "AssetDatabase.h"

struct Scene;
struct Mesh;

struct CSprite
{
    AssetHandle spriteHandle;

    REFLECT()
};

namespace SpriteDrawSystem
{	
    void Initialize();
	void OnFrame(Scene& scene, float deltaTime);
	void Destroy();
}