#pragma once

#include "Matrix.h"
#include "Vec2.h"
#include "Vec4.h"
#include "GraphicsDevice.h"
#include "Scene.h"
#include "AssetDatabase.h"

#include "Systems.h"
#include "SpatialComponent.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Scene;
struct FrameContext;

struct Character
{
	Vec2i size{ Vec2i(0, 0) };
	Vec2i bearing{ Vec2i(0, 0) };
	Vec2f UV0{ Vec2f(0.f, 0.f) };
	Vec2f UV1{ Vec2f(1.f, 1.f) };
	int advance;
};

struct TextComponent : public SpatialComponent
{
	eastl::string text;
	AssetHandle fontAsset;
	
	REFLECT_DERIVED();
};

namespace FreeType
{
	FT_Library* Get();
}

struct FontDrawSystem : public IWorldSystem
{
	~FontDrawSystem();

    virtual void Activate() override;

	virtual void RegisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void UnregisterComponent(Entity* pEntity, IComponent* pComponent) override;

	virtual void Draw(UpdateContext& ctx, FrameContext& frameCtx) override;

	static FT_Library GetFreeType();

private:
	eastl::vector<TextComponent*> textComponents;

	ProgramHandle fontShaderProgram;
	ConstBufferHandle constBuffer;
	BlendStateHandle blendState;
	VertexBufferHandle vertexBuffer;
	VertexBufferHandle texcoordsBuffer;
	IndexBufferHandle indexBuffer;
	SamplerHandle charTextureSampler;

	struct FontUniforms
	{
		Matrixf projection;
		Vec4f color;
	};
};