#include "SpriteDrawSystem.h"

#include "Scene.h"
#include "GameRenderer.h"
#include "Mesh.h"
#include "AssetDatabase.h"
#include "Image.h"
#include "Profiler.h"

struct SpriteUniforms
{
    Matrixf wvp;
};

namespace
{
    ConstBufferHandle transformBufferHandle;
    VertexBufferHandle vertBuffer;
    SamplerHandle spriteSampler;
    VertexShaderHandle vertShader;
    PixelShaderHandle pixelShader;
    ProgramHandle program;
    BlendStateHandle blendState;
}

REFLECT_COMPONENT_BEGIN(CSprite)
REFLECT_MEMBER(spriteHandle)
REFLECT_END()

// ***********************************************************************

void SpriteDrawSystem::Initialize()
{
	transformBufferHandle = GfxDevice::CreateConstantBuffer(sizeof(SpriteUniforms), "Sprite Transform Buffer");

    eastl::vector<Vert_PosTex> quadVertices = {
        Vert_PosTex{Vec3f(-1.0f, -1.0f, 0.0f), Vec2f(0.0f, 1.0f)},
        Vert_PosTex{Vec3f(1.f, -1.f, 0.0f), Vec2f(1.0f, 1.0f)},
        Vert_PosTex{Vec3f(-1.f, 1.f, 0.0f), Vec2f(0.0f, 0.0f)},
        Vert_PosTex{Vec3f(1.f, 1.f, 0.0f), Vec2f(1.0f, 0.0f)}
    };
    vertBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vert_PosTex), quadVertices.data(), "Sprite quad");

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

    eastl::vector<VertexInputElement> layout;
	layout.push_back({"POSITION", AttributeType::Float3});
	layout.push_back({"TEXCOORD", AttributeType::Float2});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(basicSpriteShader, "VSMain", layout, "Sprite Vert Shader");
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

void SpriteDrawSystem::OnFrame(Scene& scene, FrameContext& ctx, float deltaTime)
{
    PROFILE();
	
	GFX_SCOPED_EVENT("Drawing sprites");

    GfxDevice::SetTopologyType(TopologyType::TriangleStrip);
    GfxDevice::BindProgram(program);
    GfxDevice::BindVertexBuffers(1, &vertBuffer);
	GfxDevice::SetBlending(blendState);
	GfxDevice::BindSampler(spriteSampler, ShaderType::Pixel, 0);
    
    for (EntityID ent : SceneView<CSprite, CTransform>(scene))
    {
        CSprite* pSprite = scene.Get<CSprite>(ent);
        Image* pImage = AssetDB::GetAsset<Image>(pSprite->spriteHandle);
        if (pImage == nullptr)
            continue;

        CTransform* pTrans = scene.Get<CTransform>(ent);
        Matrixf wvp = ctx.projection * ctx.view * pTrans->globalTransform;
		SpriteUniforms uniformData{ wvp };
		GfxDevice::BindConstantBuffer(transformBufferHandle, &uniformData, ShaderType::Vertex, 0);

        GfxDevice::BindTexture(pImage->gpuHandle, ShaderType::Pixel, 0);

        GfxDevice::Draw(4, 0);
    }

    GfxDevice::SetBlending(INVALID_HANDLE);
}

void SpriteDrawSystem::Destroy()
{
    GfxDevice::FreeBlendState(blendState);
    GfxDevice::FreeVertexBuffer(vertBuffer);
    GfxDevice::FreeConstBuffer(transformBufferHandle);
    GfxDevice::FreeSampler(spriteSampler);
    GfxDevice::FreeVertexShader(vertShader);
    GfxDevice::FreePixelShader(pixelShader);
    GfxDevice::FreeProgram(program);
}