#pragma once

#include "GraphicsDevice.h"
#include "AssetDatabase.h"

struct Scene;
struct Mesh;
struct FrameContext;

struct CRenderable
{
    AssetHandle shaderHandle;
    AssetHandle meshHandle;

    REFLECT()
};

namespace SceneDrawSystem
{	
    void OnSceneCreate(Scene& scene);
	void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
}