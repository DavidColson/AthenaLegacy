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
struct UpdateContext;

struct Renderable : public SpatialComponent
{
	AssetHandle shaderHandle;
    AssetHandle meshHandle;
	
	REFLECT_DERIVED();
};

struct SceneDrawSystem : public IWorldSystem
{
	~SceneDrawSystem();

    virtual void Activate() override;

    virtual void Deactivate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Draw(UpdateContext& ctx, FrameContext& frameCtx) override;

	eastl::vector<Renderable*> renderableComponents;
};
