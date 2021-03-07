#pragma once

#include "GraphicsDevice.h"
#include "AssetDatabase.h"

#include "Systems.h"
#include "SpatialComponent.h"
#include "Mesh.h"

struct Scene;
struct Mesh;
struct FrameContext;

struct Sprite : public SpatialComponent
{
	AssetHandle spriteHandle;
	
	REFLECT_DERIVED();
};

struct SpriteDrawSystem : public ISystem
{
    ~SpriteDrawSystem();

    virtual void Activate() override;

	virtual void RegisterComponent(IComponent* pComponent) override;

	virtual void UnregisterComponent(IComponent* pComponent) override;

	virtual void Draw(float deltaTime, FrameContext& ctx) override;

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
