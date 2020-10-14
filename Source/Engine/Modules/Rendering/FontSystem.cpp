
#include "FontSystem.h"

#include "Matrix.h"
#include "Vec3.h"
#include "Vec2.h"
#include "Log.h"
#include "Scene.h"
#include "Profiler.h"
#include "Font.h"
#include "Mesh.h"
#include "RenderSystem.h"

#include "Imgui/imgui.h"

REFLECT_COMPONENT_BEGIN(CText)
REFLECT_MEMBER(text)
REFLECT_END()

struct FontSystemState
{
	ProgramHandle fontShaderProgram;
	ConstBufferHandle constBuffer;
	BlendStateHandle blendState;
	VertexBufferHandle vertexBuffer;
	IndexBufferHandle indexBuffer;
	SamplerHandle charTextureSampler;

	struct FontUniforms
	{
		Matrixf projection;
		Vec4f color;
	};

	FT_Library freetype;
};

namespace 
{
	FontSystemState* pState = nullptr;
}

// ***********************************************************************

FT_Library FontSystem::GetFreeType()
{
	return pState->freetype;
}

#define CHARS_PER_DRAW_CALL 500

// ***********************************************************************

void FontSystem::Initialize()
{
	pState = new FontSystemState();

	FT_Init_FreeType(&(pState->freetype));

	// FONT ASSET
	// Font asset will call this New_Face function, and store the resultant font face
	// It should also generate it's font atlas during load time, uploading the result to the GPU and storing a handle
	// The array of character data can also be generated and store in the asset file
	// I think it probably best that it create it's rendering data as well, so programs, vert buffers etc etc
	// Come to think of it, we might not even need to save the face at all

	eastl::string fontShaderSrc = "\
	cbuffer cbFontUniforms\
	{\
		float4x4 projection;\
		float4 color;\
	};\
	struct VS_OUTPUT\
	{\
		float4 Pos : SV_POSITION;\
		float2 Tex : TEXCOORD0;\
	};\
	VS_OUTPUT VSMain(float4 inPos : POSITION, float2 inTex : TEXCOORD0)\
	{\
		VS_OUTPUT output;\
		output.Pos = mul(inPos, projection);\
		output.Tex = inTex;\
		return output;\
	}\
	Texture2D shaderTexture;\
	SamplerState SampleType;\
	float4 PSMain(VS_OUTPUT input) : SV_TARGET\
	{\
		float4 textureColor = float4(1.0, 1.0, 1.0, shaderTexture.Sample(SampleType, input.Tex).r);\
		return textureColor * color;\
	}";

	eastl::vector<VertexInputElement> layout;
	layout.push_back({"POSITION", AttributeType::Float3});
	layout.push_back({"TEXCOORD", AttributeType::Float2});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(fontShaderSrc, "VSMain", layout, "Fonts");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(fontShaderSrc, "PSMain", "Fonts");
	pState->fontShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

	BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	pState->blendState = GfxDevice::CreateBlendState(blender);

	// Create vertex buffer
	// ********************
	pState->vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(CHARS_PER_DRAW_CALL * 4, sizeof(Vert_PosTex), "Font Render Vert Buffer");
	pState->indexBuffer = GfxDevice::CreateDynamicIndexBuffer(CHARS_PER_DRAW_CALL * 6, IndexFormat::UInt, "Font Render Index Buffer");

	// Create a constant buffer for the WVP
	// **********************************************

	pState->constBuffer = GfxDevice::CreateConstantBuffer(sizeof(FontSystemState::FontUniforms), "Font Uniforms");
	pState->charTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Clamp, "Font char");
}

// ***********************************************************************

void FontSystem::Destroy()
{
	GfxDevice::FreeProgram(pState->fontShaderProgram);
	GfxDevice::FreeVertexBuffer(pState->vertexBuffer);
	GfxDevice::FreeIndexBuffer(pState->indexBuffer);
	GfxDevice::FreeSampler(pState->charTextureSampler);
	GfxDevice::FreeConstBuffer(pState->constBuffer);
	GfxDevice::FreeBlendState(pState->blendState);
}

// ***********************************************************************

void FontSystem::OnFrame(Scene& scene, float /* deltaTime */)
{
	PROFILE();
	
	GFX_SCOPED_EVENT("Drawing text");

	// Set graphics state correctly
	GfxDevice::SetTopologyType(TopologyType::TriangleList);
	GfxDevice::BindProgram(pState->fontShaderProgram);
	GfxDevice::SetBlending(pState->blendState);
	GfxDevice::BindSampler(pState->charTextureSampler, ShaderType::Pixel, 0);

	for (EntityID ent : SceneView<CText, CTransform>(scene))
	{
		// Skip if these entity has a visiblity component set to false
		if (scene.Has<CVisibility>(ent))
		{
			if (scene.Get<CVisibility>(ent)->visible == false)
				continue;
		}

		CText* pText = scene.Get<CText>(ent);
		CTransform* pTransform = scene.Get<CTransform>(ent);
		Font* pFont = AssetDB::GetAsset<Font>(pText->fontAsset);

		float textWidth = 0.0f;
		float x = pTransform->localPos.x;
		float y = pTransform->localPos.y;

		for (char const& c : pText->text)
		{
			Character ch = pFont->characters[c];
			textWidth += ch.advance * pTransform->localSca.x;
		}

		eastl::fixed_vector<Vert_PosTex, CHARS_PER_DRAW_CALL * 4> vertexList;
		eastl::fixed_vector<uint32_t, CHARS_PER_DRAW_CALL * 6> indexList;
		int currentIndex = 0;

		for (char const& c : pText->text) {
			Character ch = pFont->characters[c];

			float xpos = (x + ch.bearing.x * pTransform->localSca.x) - textWidth * 0.5f;
			float ypos = y - (ch.size.y - ch.bearing.y) * pTransform->localSca.y;
			float w = (float)ch.size.x * pTransform->localSca.x;
			float h = (float)ch.size.y * pTransform->localSca.y;
	
			vertexList.push_back( Vert_PosTex{ Vec3f( xpos, ypos, 0.0f), Vec2f(ch.UV0.x, ch.UV1.y) });
			vertexList.push_back( Vert_PosTex{ Vec3f( xpos + w, ypos + h, 0.0f), Vec2f(ch.UV1.x, ch.UV0.y) });
			vertexList.push_back( Vert_PosTex{ Vec3f( xpos, ypos + h, 0.0f), Vec2f(ch.UV0.x, ch.UV0.y) });
			vertexList.push_back( Vert_PosTex{ Vec3f( xpos + w, ypos, 0.0f), Vec2f(ch.UV1.x, ch.UV1.y) });
			
			// First triangle
			indexList.push_back(currentIndex);
			indexList.push_back(currentIndex + 1);
			indexList.push_back(currentIndex + 2);

			// Second triangle
			indexList.push_back(currentIndex);
			indexList.push_back(currentIndex + 3);
			indexList.push_back(currentIndex + 1);
			currentIndex += 4; // move along by 4 vertices for the next character

			x += ch.advance * pTransform->localSca.x;
		}
	
		// Update buffers
		GfxDevice::UpdateDynamicVertexBuffer(pState->vertexBuffer, vertexList.data(), vertexList.size() * sizeof(Vert_PosTex));
		GfxDevice::UpdateDynamicIndexBuffer(pState->indexBuffer, indexList.data(), indexList.size() * sizeof(uint32_t));

		GfxDevice::BindVertexBuffers(1, &pState->vertexBuffer);
		GfxDevice::BindIndexBuffer(pState->indexBuffer);
		
		GfxDevice::BindTexture(pFont->fontTexture, ShaderType::Pixel, 0);

		Matrixf projection = Matrixf::Orthographic(0, RenderSystem::GetWidth(), 0.0f, RenderSystem::GetHeight(), 0.1f, 10.0f); // transform into screen space
		FontSystemState::FontUniforms uniformData{ projection, Vec4f(1.0f, 1.0f, 1.0f, 1.0f) };
		GfxDevice::BindConstantBuffer(pState->constBuffer, &uniformData, ShaderType::Vertex, 0);
		GfxDevice::BindConstantBuffer(pState->constBuffer, &uniformData, ShaderType::Pixel, 0);
		
		// Do draw call for this text
		GfxDevice::DrawIndexed((int)indexList.size(), 0, 0);
	}
	GfxDevice::SetBlending(INVALID_HANDLE);
}