
#include "FontSystem.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Matrix.h"
#include "Vec3.h"
#include "Vec2.h"
#include "Log.h"
#include "Scene.h"
#include "Profiler.h"
#include "RectPacking.h"

#include "Imgui/imgui.h"

REFLECT_BEGIN(CText)
REFLECT_MEMBER(text)
REFLECT_END()

namespace 
{
	FT_Library freetype;
	FT_Face face;
	bool startedFreeType = false;
}

#define CHARS_PER_DRAW_CALL 500

void FontSystem::OnAddFontSystemState(Scene& scene, EntityID entity)
{
	CFontSystemState& state = *(scene.Get<CFontSystemState>(entity));

	if (startedFreeType == false)
	{
		FT_Init_FreeType(&freetype);

		FT_Error err = FT_New_Face(freetype, "Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 0, &face);
		if (err)
		{
			Log::Warn("FreeType Error: %s", FT_Error_String(err));
		}
		startedFreeType = true;
	}

	std::string fontShaderSrc = "\
	cbuffer cbTransform\
	{\
		float4x4 projection;\
	};\
	struct VS_OUTPUT\
	{\
		float4 Pos : SV_POSITION;\
		float4 Col : COLOR;\
		float2 Tex : TEXCOORD0;\
	};\
	VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR, float2 inTex : TEXCOORD0)\
	{\
		VS_OUTPUT output;\
		output.Pos = mul(inPos, projection);\
		output.Col = inCol;\
		output.Tex = inTex;\
		return output;\
	}\
	Texture2D shaderTexture;\
	SamplerState SampleType;\
	float4 PSMain(VS_OUTPUT input) : SV_TARGET\
	{\
		float4 textureColor;\
		textureColor = float4(1.0, 1.0, 1.0, shaderTexture.Sample(SampleType, input.Tex).r);\
		return textureColor;\
	}";

	std::vector<VertexInputElement> layout;
	layout.push_back({"POSITION", AttributeType::Float3});
	layout.push_back({"COLOR", AttributeType::Float3});
	layout.push_back({"TEXCOORD", AttributeType::Float2});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(fontShaderSrc, "VSMain", layout, "Fonts");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(fontShaderSrc, "PSMain", "Fonts");

	state.fontShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

	BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	state.blendState = GfxDevice::CreateBlendState(blender);

	// Create vertex buffer
	// ********************
	state.vertexBuffer = GfxDevice::CreateDynamicVertexBuffer(CHARS_PER_DRAW_CALL * 4, sizeof(Vertex), "Font Render Vert Buffer");
	state.indexBuffer = GfxDevice::CreateDynamicIndexBuffer(CHARS_PER_DRAW_CALL * 6, "Font Render Index Buffer");

	// Create a constant buffer for the WVP
	// **********************************************

	state.wvpBuffer = GfxDevice::CreateConstantBuffer(sizeof(CFontSystemState::TransformData), "Font transforms");
	state.charTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Clamp, "Font char");

	// Rasterize the entire font to a texture atlas
	// **********************************************

	// Texture data
	int texHeight = 512;
	int texWidth = 512;
	uint8_t* pTextureDataAsR8{ nullptr };
	pTextureDataAsR8 = new uint8_t[texHeight * texWidth];
	memset(pTextureDataAsR8, 0, texHeight * texWidth);

	eastl::vector<Packing::Rect> rects;
	rects.get_allocator().set_name("Font Packing Rects");
	state.characters.get_allocator().set_name("Font character data");

	FT_Set_Pixel_Sizes(face, 0, 50);

	// Prepare rects for packing
	for (int i = 0; i < 128; i++)
	{
		FT_Load_Char(face, i, FT_LOAD_DEFAULT);
		Packing::Rect newRect;
		newRect.w = face->glyph->bitmap.width + 1;
		newRect.h = face->glyph->bitmap.rows + 1;
		rects.push_back(newRect);
	}
	Packing::SkylinePackRects(rects, texWidth, texHeight);

	for (int i = 0; i < 128; i++)
	{
		Packing::Rect& rect = rects[i];
		FT_Load_Char(face, i, FT_LOAD_RENDER);

		// Create the character with all it's appropriate data
		Character character;
		character.size = Vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.bearing = Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.advance = (face->glyph->advance.x) >> 6;
		character.UV0 = Vec2f((float)rect.x / (float)texWidth, (float)rect.y / (float)texHeight);
		character.UV1 = Vec2f((float)(rect.x + character.size.x) / (float)texWidth, (float)(rect.y + character.size.y) / (float)texHeight);
		state.characters.push_back(character);

		// Blit the glyph's image into our texture atlas
		uint8_t* pSourceData = face->glyph->bitmap.buffer;
		uint8_t* pDestination = pTextureDataAsR8 + rect.y * texWidth + rect.x;
		int sourceDataPitch = face->glyph->bitmap.pitch;
		for (uint32_t y = 0; y < face->glyph->bitmap.rows; y++, pSourceData += sourceDataPitch, pDestination += texWidth)
		{
			memcpy(pDestination, pSourceData, face->glyph->bitmap.width);
		}
	}

	state.fontTexture = GfxDevice::CreateTexture(texWidth, texHeight, TextureFormat::R8, pTextureDataAsR8, "Font Atlas");
	delete[] pTextureDataAsR8;
}

void FontSystem::OnRemoveFontSystemState(Scene& scene, EntityID entity)
{
	CFontSystemState& state = *(scene.Get<CFontSystemState>(entity));

	GfxDevice::FreeProgram(state.fontShaderProgram);
	GfxDevice::FreeVertexBuffer(state.vertexBuffer);
	GfxDevice::FreeIndexBuffer(state.indexBuffer);
	GfxDevice::FreeSampler(state.charTextureSampler);
	GfxDevice::FreeConstBuffer(state.wvpBuffer);
	GfxDevice::FreeBlendState(state.blendState);
	GfxDevice::FreeTexture(state.fontTexture);
}

void FontSystem::OnFrame(Scene& scene, float /* deltaTime */)
{
	PROFILE();
	
	GFX_SCOPED_EVENT("Drawing text");
	
	CFontSystemState& state = *(scene.Get<CFontSystemState>(ENGINE_SINGLETON));

	// Set graphics state correctly
	GfxDevice::SetTopologyType(TopologyType::TriangleList);
	GfxDevice::BindProgram(state.fontShaderProgram);
	GfxDevice::SetBlending(state.blendState);
	GfxDevice::BindSampler(state.charTextureSampler, ShaderType::Pixel, 0);

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

		float textWidth = 0.0f;
		float x = pTransform->pos.x;
		float y = pTransform->pos.y;

		for (char const& c : pText->text)
		{
			Character ch = state.characters[c];
			textWidth += ch.advance * pTransform->sca.x;
		}

		eastl::fixed_vector<Vertex, CHARS_PER_DRAW_CALL * 4> vertexList;
		eastl::fixed_vector<int, CHARS_PER_DRAW_CALL * 6> indexList;
		int currentIndex = 0;

		for (char const& c : pText->text) {
			Character ch = state.characters[c];

			float xpos = (x + ch.bearing.x * pTransform->sca.x) - textWidth * 0.5f;
			float ypos = y - (ch.size.y - ch.bearing.y) * pTransform->sca.y;
			float w = (float)ch.size.x * pTransform->sca.x;
			float h = (float)ch.size.y * pTransform->sca.y;
	
			vertexList.push_back( Vertex{ Vec3f( xpos, ypos, 1.0f), 		Vec3f(1.0f, 1.0f, 1.0f), Vec2f(ch.UV0.x, ch.UV1.y) });
			vertexList.push_back( Vertex{ Vec3f( xpos, ypos + h, 1.0f), 	Vec3f(1.0f, 1.0f, 1.0f), Vec2f(ch.UV0.x, ch.UV0.y) });
			vertexList.push_back( Vertex{ Vec3f( xpos + w, ypos + h, 1.0f), Vec3f(1.0f, 1.0f, 1.0f), Vec2f(ch.UV1.x, ch.UV0.y) });
			vertexList.push_back( Vertex{ Vec3f( xpos + w, ypos, 1.0f), 	Vec3f(1.0f, 1.0f, 1.0f), Vec2f(ch.UV1.x, ch.UV1.y) });
			
			// First triangle
			indexList.push_back(currentIndex);
			indexList.push_back(currentIndex + 2);
			indexList.push_back(currentIndex + 3);

			// Second triangle
			indexList.push_back(currentIndex);
			indexList.push_back(currentIndex + 1);
			indexList.push_back(currentIndex + 2);
			currentIndex += 4; // move along by 4 vertices for the next character

			x += ch.advance * pTransform->sca.x;
		}
	
		// Update buffers
		GfxDevice::UpdateDynamicVertexBuffer(state.vertexBuffer, vertexList.data(), vertexList.size() * sizeof(Vertex));
		GfxDevice::UpdateDynamicIndexBuffer(state.indexBuffer, indexList.data(), indexList.size() * sizeof(int));

		GfxDevice::BindVertexBuffers(1, &state.vertexBuffer);
		GfxDevice::BindIndexBuffer(state.indexBuffer);
		
		GfxDevice::BindTexture(state.fontTexture, ShaderType::Pixel, 0);

		Matrixf projection = Matrixf::Orthographic(0, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), 0.1f, 10.0f); // transform into screen space
		CFontSystemState::TransformData transformData{ projection };
		GfxDevice::BindConstantBuffer(state.wvpBuffer, &transformData, ShaderType::Vertex, 0);
		
		// Do draw call for this text
		GfxDevice::DrawIndexed((int)indexList.size(), 0, 0);
	}
}