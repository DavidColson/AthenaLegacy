
#include "RenderFont.h"

#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Maths/Matrix.h"
#include "Maths/Vec3.h"
#include "RenderProxy.h"
#include "Log.h"
#include "Renderer.h"

RenderFont::RenderFont(std::string fontFile, int size)
{
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

	fontShader = Graphics::LoadShaderFromText(fontShaderSrc);


	std::vector<Vertex> quadVertices = {
		Vertex(Vec3f(0.0f, 0.0f, 0.5f)),
		Vertex(Vec3f(0.f, 10.f, 0.5f)),
		Vertex(Vec3f(10.f, 10.f, 0.5f)),
		Vertex(Vec3f(10.f, 0.f, 0.5f))
	};
	std::vector<int> quadIndices = {
		0, 1, 2,
		3, 0
	};

	quadVertices[0].texCoords = Vec2f(0.0f, 1.0f);
	quadVertices[1].texCoords = Vec2f(0.0f, 0.0f);
	quadVertices[2].texCoords = Vec2f(1.0f, 0.0f);
	quadVertices[3].texCoords = Vec2f(1.0f, 1.0f);

	// Create vertex buffer
	// ********************

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * UINT(quadVertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// fill the buffer with actual data
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = quadVertices.data();

	Graphics::GetContext()->pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pQuadVertBuffer);

	// Create an index buffer
	// **********************

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * UINT(quadIndices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	// fill the index buffer with actual data
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = quadIndices.data();
	Graphics::GetContext()->pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &pQuadIndexBuffer);

	// Create a constant buffer (uniform) for the WVP
	// **********************************************

	D3D11_BUFFER_DESC wvpBufferDesc;
	ZeroMemory(&wvpBufferDesc, sizeof(wvpBufferDesc));
	wvpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	wvpBufferDesc.ByteWidth = sizeof(cbTransform);
	wvpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wvpBufferDesc.CPUAccessFlags = 0;
	wvpBufferDesc.MiscFlags = 0;
	Graphics::GetContext()->pDevice->CreateBuffer(&wvpBufferDesc, nullptr, &pQuadWVPBuffer);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	Graphics::GetContext()->pDevice->CreateBlendState(&blendDesc, &transparency);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Graphics::GetContext()->pDevice->CreateSamplerState(&sampDesc, &charTextureSampler);

	for (int i = 0; i < 128; i++)
	{
		FT_Face face;
		FT_New_Face(freetype, fontFile.c_str(), 0, &face);

		FT_Set_Pixel_Sizes(face, 0, size);

		FT_Load_Char(face, i, FT_LOAD_RENDER);

		Character character;
		if (face->glyph->bitmap.width > 0 && face->glyph->bitmap.rows > 0)
		{
			Graphics::Texture2D texture = Graphics::CreateTexture2D(
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				DXGI_FORMAT_R8_UNORM,
				face->glyph->bitmap.buffer,
				D3D11_BIND_SHADER_RESOURCE
			);

			character.charTexture = texture.pShaderResourceView;
		}
		character.size = Vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.bearing = Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.advance = (face->glyph->advance.x) >> 6;

		int size = sizeof(character);
		int size2 = sizeof(character.size);

		characters.push_back(character);
	}
}

void RenderFont::SubmitText(const char* text, Vec2f pos)
{
	textQueue.emplace_back(text, pos);
}

void RenderFont::DrawQueue()
{
	// Set vertex buffer as active
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::GetContext()->pDeviceContext->IASetVertexBuffers(0, 1, &pQuadVertBuffer, &stride, &offset);

	Graphics::GetContext()->pDeviceContext->IASetIndexBuffer(pQuadIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Graphics::GetContext()->pDeviceContext->VSSetConstantBuffers(0, 1, &(pQuadWVPBuffer));

	// Set Shaders to active
	Graphics::GetContext()->pDeviceContext->VSSetShader(fontShader.pVertexShader, 0, 0);
	Graphics::GetContext()->pDeviceContext->PSSetShader(fontShader.pPixelShader, 0, 0);
	Graphics::GetContext()->pDeviceContext->GSSetShader(fontShader.pGeometryShader, 0, 0);

	Graphics::GetContext()->pDeviceContext->IASetInputLayout(fontShader.pVertLayout);

	Graphics::GetContext()->pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	float blendFactor[] = { 0.0f, 0.f, 0.0f, 0.0f };
	Graphics::GetContext()->pDeviceContext->OMSetBlendState(transparency, blendFactor, 0xffffffff);

	Matrixf projection = Matrixf::Orthographic(0, Graphics::GetContext()->windowWidth / Graphics::GetContext()->pixelScale, 0.0f, Graphics::GetContext()->windowHeight / Graphics::GetContext()->pixelScale, 0.1f, 10.0f); // transform into screen space
	
	Graphics::GetContext()->pDeviceContext->PSSetSamplers(0, 1, &charTextureSampler);

	for (QueueElement const& element : textQueue)
	{
		float textWidth = 0.0f;
		float x = element.pos.x;
		float y = element.pos.y;


		for (char const& c : element.text)
		{
			Character ch = characters[c];
			textWidth += ch.advance;
		}

		for (char const& c : element.text) {
			// Draw a font character
			Character ch = characters[c];

			Matrixf posmat = Matrixf::Translate(Vec3f(float(x + ch.bearing.x - textWidth*0.5f), float(y - (ch.size.y - ch.bearing.y)), 0.0f));
			Matrixf scalemat = Matrixf::Scale(Vec3f(ch.size.x / 10.0f, ch.size.y / 10.0f, 1.0f));

			Matrixf world = posmat * scalemat; // transform into world space
			Matrixf wvp = projection * world;

			cbCharTransform.wvp = wvp;
			Graphics::GetContext()->pDeviceContext->UpdateSubresource(pQuadWVPBuffer, 0, nullptr, &cbCharTransform, 0, 0);

			Graphics::GetContext()->pDeviceContext->PSSetShaderResources(0, 1, &(characters[c].charTexture));

			// do 3D rendering on the back buffer here
			// Instance render the entire string
			Graphics::GetContext()->pDeviceContext->DrawIndexed(5, 0, 0);

			x += ch.advance;
		}
	}
	textQueue.clear();
}

