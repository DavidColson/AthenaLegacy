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

struct CCamera
{
    float fov{ 60.0f };
    float horizontalAngle{ 0.0f };
    float verticalAngle{ 0.0f };

    REFLECT()
};

namespace SceneDrawSystem
{	
    void OnSceneCreate(Scene& scene);
	void OnFrame(Scene& scene, FrameContext& ctx, float deltaTime);
}