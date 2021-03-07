
#include "FontSystem.h"

#include "Matrix.h"
#include "Vec3.h"
#include "Vec2.h"
#include "Log.h"
#include "Scene.h"
#include "Profiler.h"
#include "Font.h"
#include "Mesh.h"
#include "GameRenderer.h"

#include "Imgui/imgui.h"

REFLECT_COMPONENT_BEGIN(CText)
REFLECT_MEMBER(text)
REFLECT_END()

REFLECT_COMPONENT_BEGIN(TextComponent)
REFLECT_MEMBER(text)
REFLECT_MEMBER(fontAsset)
REFLECT_END()

#define CHARS_PER_DRAW_CALL 500

namespace
{
	FT_Library* pFreetype;
}

FT_Library* FreeType::Get()
{
	return pFreetype;
}

// ***********************************************************************

void FontDrawSystem::Activate()
{
	GameRenderer::RegisterRenderSystemTransparent(this);

	pFreetype = new FT_Library();
	FT_Init_FreeType(pFreetype);

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
		float4 outColor = color * float4(1.0, 1.0, 1.0, shaderTexture.Sample(SampleType, input.Tex).r);\
		clip(outColor.a - 0.00001);\
		return outColor;\
	}";

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(fontShaderSrc, "VSMain", "Fonts");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(fontShaderSrc, "PSMain", "Fonts");
	fontShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

	BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	blender.sourceAlpha = Blend::InverseSrcAlpha;
	blender.destinationAlpha = Blend::One;
	blendState = GfxDevice::CreateBlendState(blender);

	// Create vertex buffers
	// ********************

	vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(CHARS_PER_DRAW_CALL * 4, sizeof(Vec3f), "Font Render Vert Buffer");
	texcoordsBuffer = GfxDevice::CreateDynamicVertexBuffer(CHARS_PER_DRAW_CALL * 4, sizeof(Vec2f), "Font Render Texcoords Buffer");
	indexBuffer = GfxDevice::CreateDynamicIndexBuffer(CHARS_PER_DRAW_CALL * 6, IndexFormat::UInt, "Font Render Index Buffer");

	// Create a constant buffer for the WVP
	// **********************************************

	constBuffer = GfxDevice::CreateConstantBuffer(sizeof(FontUniforms), "Font Uniforms");
	charTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Clamp, "Font char");
}

// ***********************************************************************

void FontDrawSystem::RegisterComponent(IComponent* pComponent)
{
	if (pComponent->GetTypeData() == TypeDatabase::Get<TextComponent>())
	{
		textComponents.push_back(static_cast<TextComponent*>(pComponent));
	}
}

// ***********************************************************************

void FontDrawSystem::UnregisterComponent(IComponent* pComponent)
{
	eastl::vector<TextComponent*>::iterator found = eastl::find(textComponents.begin(), textComponents.end(), pComponent);
	if (found != textComponents.end())
	{
		textComponents.erase(found);
	}
}

// ***********************************************************************

FontDrawSystem::~FontDrawSystem()
{
	GfxDevice::FreeProgram(fontShaderProgram);
	GfxDevice::FreeVertexBuffer(vertexBuffer);
	GfxDevice::FreeVertexBuffer(texcoordsBuffer);
	GfxDevice::FreeIndexBuffer(indexBuffer);
	GfxDevice::FreeSampler(charTextureSampler);
	GfxDevice::FreeConstBuffer(constBuffer);
	GfxDevice::FreeBlendState(blendState);
}

// ***********************************************************************

void FontDrawSystem::Draw(float deltaTime, FrameContext& ctx)
{
	PROFILE();
	
	GFX_SCOPED_EVENT("Drawing text");

	// Set graphics state correctly
	GfxDevice::SetTopologyType(TopologyType::TriangleList);
	GfxDevice::BindProgram(fontShaderProgram);
	GfxDevice::SetBlending(blendState);
	GfxDevice::BindSampler(charTextureSampler, ShaderType::Pixel, 0);

	for(TextComponent* pText : textComponents)
	{
		Font* pFont = AssetDB::GetAsset<Font>(pText->fontAsset);

		Vec3f position;
		Vec3f rotation;
		Vec3f scale;
		pText->GetWorldTransform().ToTRS(position, rotation, scale);

		float textWidth = 0.0f;
		float x = position.x;
		float y = position.y;

		for (char const& c : pText->text)
		{
			Character ch = pFont->characters[c];
			textWidth += ch.advance * scale.x;
		}

		eastl::fixed_vector<Vec3f, CHARS_PER_DRAW_CALL * 4> vertexList;
		eastl::fixed_vector<Vec2f, CHARS_PER_DRAW_CALL * 4> texcoordsList;
		eastl::fixed_vector<uint32_t, CHARS_PER_DRAW_CALL * 6> indexList;
		int currentIndex = 0;

		for (char const& c : pText->text) {
			Character ch = pFont->characters[c];

			float xpos = (x + ch.bearing.x * scale.x) - textWidth * 0.5f;
			float ypos = y - (ch.size.y - ch.bearing.y) * scale.y;
			float w = (float)ch.size.x * scale.x;
			float h = (float)ch.size.y * scale.y;
	
			vertexList.push_back( Vec3f( xpos, ypos, 0.0f));
			texcoordsList.push_back( Vec2f(ch.UV0.x, ch.UV1.y));

			vertexList.push_back( Vec3f( xpos + w, ypos + h, 0.0f));
			texcoordsList.push_back( Vec2f(ch.UV1.x, ch.UV0.y));

			vertexList.push_back( Vec3f( xpos, ypos + h, 0.0f));
			texcoordsList.push_back( Vec2f(ch.UV0.x, ch.UV0.y));

			vertexList.push_back( Vec3f( xpos + w, ypos, 0.0f));
			texcoordsList.push_back( Vec2f(ch.UV1.x, ch.UV1.y));
			
			// First triangle
			indexList.push_back(currentIndex);
			indexList.push_back(currentIndex + 1);
			indexList.push_back(currentIndex + 2);

			// Second triangle
			indexList.push_back(currentIndex);
			indexList.push_back(currentIndex + 3);
			indexList.push_back(currentIndex + 1);
			currentIndex += 4; // move along by 4 vertices for the next character

			x += ch.advance * scale.x;
		}
	
		// Update buffers
		GfxDevice::UpdateDynamicVertexBuffer(vertexBuffer, vertexList.data(), vertexList.size() * sizeof(Vec3f));
		GfxDevice::UpdateDynamicVertexBuffer(texcoordsBuffer, texcoordsList.data(), texcoordsList.size() * sizeof(Vec2f));
		GfxDevice::UpdateDynamicIndexBuffer(indexBuffer, indexList.data(), indexList.size() * sizeof(uint32_t));

		GfxDevice::BindVertexBuffers(0, 1, &vertexBuffer);
		GfxDevice::BindVertexBuffers(1, 1, &texcoordsBuffer);
		GfxDevice::BindIndexBuffer(indexBuffer);
		
		GfxDevice::BindTexture(pFont->fontTexture, ShaderType::Pixel, 0);

		FontUniforms uniformData{ ctx.projection * ctx.view, Vec4f(1.0f, 1.0f, 1.0f, 1.0f) };
		GfxDevice::BindConstantBuffer(constBuffer, &uniformData, ShaderType::Vertex, 0);
		GfxDevice::BindConstantBuffer(constBuffer, &uniformData, ShaderType::Pixel, 0);
		
		// Do draw call for this text
		GfxDevice::DrawIndexed((int)indexList.size(), 0, 0);
	}
	GfxDevice::SetBlending(INVALID_HANDLE);
}