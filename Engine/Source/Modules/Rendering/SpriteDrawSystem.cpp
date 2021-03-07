#include "SpriteDrawSystem.h"

#include "Scene.h"
#include "GameRenderer.h"
#include "AssetDatabase.h"
#include "Image.h"
#include "Profiler.h"

struct SpriteUniforms
{
    Matrixf wvp;
};

REFLECT_COMPONENT_BEGIN(Sprite)
REFLECT_MEMBER(spriteHandle)
REFLECT_END()

// ***********************************************************************

void SpriteDrawSystem::Activate()
{
    GameRenderer::RegisterRenderSystemTransparent(this);

    quadPrim = Primitive::NewPlainQuad();
	transformBufferHandle = GfxDevice::CreateConstantBuffer(sizeof(SpriteUniforms), "Sprite Transform Buffer");
    spriteSampler = GfxDevice::CreateSampler(Filter::Point, WrapMode::Wrap, "Sprite sampler");

    eastl::string basicSpriteShader = "\
    cbuffer SpriteUniforms\
	{\
		float4x4 wvp;\
	};\
	struct VS_OUTPUT\
	{\
		float4 Pos : SV_POSITION;\
		float2 Tex : TEXCOORD0;\
	};\
	VS_OUTPUT VSMain(float4 inPos : POSITION, float2 inTex : TEXCOORD0)\
	{\
		VS_OUTPUT output;\
		output.Pos = mul(inPos, wvp);\
        output.Tex = inTex;\
		return output;\
	}\
	Texture2D shaderTexture;\
	SamplerState textureSampler;\
	float4 PSMain(VS_OUTPUT input) : SV_TARGET\
	{\
		return shaderTexture.Sample(textureSampler, input.Tex);\
	}";

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(basicSpriteShader, "VSMain", "Sprite Vert Shader");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(basicSpriteShader, "PSMain", "Sprite Pixel Shader");
	program = GfxDevice::CreateProgram(vertShader, pixShader);

    BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	blender.sourceAlpha = Blend::InverseSrcAlpha;
	blender.destinationAlpha = Blend::One;
	blendState = GfxDevice::CreateBlendState(blender);
}

// ***********************************************************************

void SpriteDrawSystem::RegisterComponent(IComponent* pComponent)
{
    if (pComponent->GetTypeData() == TypeDatabase::Get<Sprite>())
	{
		spriteComponents.push_back(static_cast<Sprite*>(pComponent));
	}
}

// ***********************************************************************

void SpriteDrawSystem::UnregisterComponent(IComponent* pComponent)
{
    eastl::vector<Sprite*>::iterator found = eastl::find(spriteComponents.begin(), spriteComponents.end(), pComponent);
	if (found != spriteComponents.end())
	{
		spriteComponents.erase(found);
	}
}

// ***********************************************************************

void SpriteDrawSystem::Draw(float deltaTime, FrameContext& ctx)
{
    PROFILE();
	
	GFX_SCOPED_EVENT("Drawing sprites");

    GfxDevice::SetTopologyType(TopologyType::TriangleStrip);
    GfxDevice::BindProgram(program);
    GfxDevice::BindVertexBuffers(0, 1, &quadPrim.bufferHandle_vertices);
    GfxDevice::BindVertexBuffers(1, 1, &quadPrim.bufferHandle_uv0);
	GfxDevice::SetBlending(blendState);
	GfxDevice::BindSampler(spriteSampler, ShaderType::Pixel, 0);
    
    for (Sprite* pSprite : spriteComponents)
    {
        Image* pImage = AssetDB::GetAsset<Image>(pSprite->spriteHandle);
        if (pImage == nullptr)
            continue;

        Matrixf wvp = ctx.projection * ctx.view * pSprite->GetWorldTransform();
		SpriteUniforms uniformData{ wvp };
		GfxDevice::BindConstantBuffer(transformBufferHandle, &uniformData, ShaderType::Vertex, 0);

        GfxDevice::BindTexture(pImage->gpuHandle, ShaderType::Pixel, 0);

        GfxDevice::Draw(4, 0);
    }

    GfxDevice::SetBlending(INVALID_HANDLE);
}

// ***********************************************************************

SpriteDrawSystem::~SpriteDrawSystem()
{
    GameRenderer::UnregisterRenderSystemTransparent(this);

    GfxDevice::FreeBlendState(blendState);
    GfxDevice::FreeConstBuffer(transformBufferHandle);
    GfxDevice::FreeSampler(spriteSampler);
    GfxDevice::FreeVertexShader(vertShader);
    GfxDevice::FreePixelShader(pixelShader);
    GfxDevice::FreeProgram(program);
}