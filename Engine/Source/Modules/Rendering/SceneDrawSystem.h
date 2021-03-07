#pragma once

#include "GraphicsDevice.h"
#include "AssetDatabase.h"

#include "Systems.h"
#include "Entity.h"
#include "SpatialComponent.h"

struct IComponent;

struct Scene;
struct Mesh;
struct FrameContext;

struct Renderable : public SpatialComponent
{
	AssetHandle shaderHandle;
    AssetHandle meshHandle;
	
	REFLECT_DERIVED();
};

struct SceneDrawSystem : public ISystem
{
    virtual void Initialize() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Draw(float deltaTime, FrameContext& ctx) override;

	eastl::vector<Renderable*> renderableComponents;
};
