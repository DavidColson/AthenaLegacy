
#include "FontSystem.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Matrix.h"
#include "Vec3.h"
#include "Log.h"
#include "Scene.h"
#include "Profiler.h"

REFLECT_BEGIN(CText)
REFLECT_MEMBER(text)
REFLECT_END()

void FontSystem::OnAddFontSystemState(Scene& scene, EntityID entity)
{
	CFontSystemState& state = *(scene.Get<CFontSystemState>(entity));

	FT_Library freetype;
	FT_Init_FreeType(&freetype);

	std::string fontShaderSrc = "\
	cbuffer cbTransform\
	{\
		float4x4 WVP;\
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
		output.Pos = mul(inPos, WVP);\
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

	std::vector<Vertex> quadVertices = {
		Vertex(Vec3f(0.0f, 0.0f, 0.5f)),
		Vertex(Vec3f(0.f, 10.f, 0.5f)),
		Vertex(Vec3f(10.f, 0.f, 0.5f)),
		Vertex(Vec3f(10.f, 10.f, 0.5f))
	};

	quadVertices[0].texCoords = Vec2f(0.0f, 1.0f);
	quadVertices[1].texCoords = Vec2f(0.0f, 0.0f);
	quadVertices[2].texCoords = Vec2f(1.0f, 1.0f);
	quadVertices[3].texCoords = Vec2f(1.0f, 0.0f);

	// Create vertex buffer
	// ********************

	state.quadBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vertex), quadVertices.data(), "Font Quad");

	// Create a constant buffer for the WVP
	// **********************************************

	state.wvpBuffer = GfxDevice::CreateConstantBuffer(sizeof(CFontSystemState::TransformData), "Font transforms");
	state.charTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Clamp, "Font char");

	for (int i = 0; i < 128; i++)
	{
		FT_Face face;
		FT_New_Face(freetype, "Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 0, &face);

		FT_Set_Pixel_Sizes(face, 0, 50);

		FT_Load_Char(face, i, FT_LOAD_RENDER);

		Character character;
		if (face->glyph->bitmap.width > 0 && face->glyph->bitmap.rows > 0)
		{
			std::string debugName = "Character ";
			debugName.push_back(char(i));
			character.charTexture = GfxDevice::CreateTexture(
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				TextureFormat::R8,
				face->glyph->bitmap.buffer,
				debugName
			);
		}
		character.size = Vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.bearing = Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.advance = (face->glyph->advance.x) >> 6;

		int size = sizeof(character);
		int size2 = sizeof(character.size);

		state.characters.push_back(character);
	}
}

void FontSystem::OnFrame(Scene& scene, float deltaTime)
{
	PROFILE();
	
	GFX_SCOPED_EVENT("Drawing text");
	
	CFontSystemState& state = *(scene.Get<CFontSystemState>(ENGINE_SINGLETON));

	GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

	// Set vertex buffer as active
	GfxDevice::BindVertexBuffers(1, &state.quadBuffer);

	// Set Shaders to active
	GfxDevice::BindProgram(state.fontShaderProgram);

	BlendingInfo blender;
	blender.enabled = true;
	blender.source = Blend::SrcAlpha;
	blender.destination = Blend::InverseSrcAlpha;
	GfxDevice::SetBlending(blender);

	Matrixf projection = Matrixf::Orthographic(0, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), 0.1f, 10.0f); // transform into screen space
	
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
			textWidth += ch.advance;
		}

		for (char const& c : pText->text) {
			// Draw a font character
			Character ch = state.characters[c];

			if (IsValid(state.characters[c].charTexture))
			{
				Matrixf posmat = Matrixf::Translate(Vec3f(float(x + ch.bearing.x - textWidth * 0.5f), float(y - (ch.size.y - ch.bearing.y)), 0.0f));
				Matrixf scalemat = Matrixf::Scale(Vec3f(ch.size.x / 10.0f, ch.size.y / 10.0f, 1.0f));

				Matrixf world = posmat * scalemat; // transform into world space
				Matrixf wvp = projection * world;

				CFontSystemState::TransformData transformData{ wvp };
				GfxDevice::BindConstantBuffer(state.wvpBuffer, &transformData, ShaderType::Vertex, 0);
				GfxDevice::BindTexture(state.characters[c].charTexture, ShaderType::Pixel, 0);

				// do 3D rendering on the back buffer here
				// Todo::Instance render the entire string
				GfxDevice::Draw(4, 0);
			}

			x += ch.advance;
		}
	}
}