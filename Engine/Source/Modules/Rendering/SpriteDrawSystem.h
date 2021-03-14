#pragma once

#include "GraphicsDevice.h"
#include "AssetDatabase.h"

#include "Systems.h"
#include "SpatialComponent.h"
#include "Mesh.h"

struct Scene;
struct Mesh;
struct FrameContext;
struct UpdateContext;

struct Sprite : public SpatialComponent
{
	AssetHandle spriteHandle;
	
	REFLECT_DERIVED();
};

struct SpriteDrawSystem : public IWorldSystem
{
    ~SpriteDrawSystem();

    virtual void Activate() override;

    virtual void Deactivate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Draw(UpdateContext& ctx, FrameContext& frameCtx) override;

private:
    ConstBufferHandle transformBufferHandle;
    SamplerHandle spriteSampler;
    VertexShaderHandle vertShader;
    PixelShaderHandle pixelShader;
    ProgramHandle program;
    BlendStateHandle blendState;

    Primitive quadPrim;

	eastl::vector<Sprite*> spriteComponents;
};
