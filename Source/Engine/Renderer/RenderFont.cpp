
#include "RenderFont.h"

#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Maths/Matrix.h"
#include "Maths/Vec3.h"
#include "RenderProxy.h"
#include "Renderer.h"

RenderFont::RenderFont(std::string fontFile, int size)
{
	FT_Library freetype;
	FT_Init_FreeType(&freetype);

	std::string fontShader = "\
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

	m_fontShader = Graphics::LoadShaderFromText(fontShader);


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

	quadVertices[0].m_texCoords = Vec2f(0.0f, 1.0f);
	quadVertices[1].m_texCoords = Vec2f(0.0f, 0.0f);
	quadVertices[2].m_texCoords = Vec2f(1.0f, 0.0f);
	quadVertices[3].m_texCoords = Vec2f(1.0f, 1.0f);

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

	Graphics::GetContext()->m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_pQuadVertBuffer);

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
	Graphics::GetContext()->m_pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_pQuadIndexBuffer);

	// Create a constant buffer (uniform) for the WVP
	// **********************************************

	D3D11_BUFFER_DESC wvpBufferDesc;
	ZeroMemory(&wvpBufferDesc, sizeof(wvpBufferDesc));
	wvpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	wvpBufferDesc.ByteWidth = sizeof(cbTransform);
	wvpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wvpBufferDesc.CPUAccessFlags = 0;
	wvpBufferDesc.MiscFlags = 0;
	Graphics::GetContext()->m_pDevice->CreateBuffer(&wvpBufferDesc, nullptr, &m_pQuadWVPBuffer);

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

	Graphics::GetContext()->m_pDevice->CreateBlendState(&blendDesc, &m_transparency);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Graphics::GetContext()->m_pDevice->CreateSamplerState(&sampDesc, &m_charTextureSampler);

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

			character.m_charTexture = texture.m_pShaderResourceView;
		}
		character.m_size = Vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.m_bearing = Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.m_advance = (face->glyph->advance.x) >> 6;

		int size = sizeof(character);
		int size2 = sizeof(character.m_size);

		m_characters.push_back(character);
	}
}

void RenderFont::Draw(std::string text, int x, int y)
{
	// Set vertex buffer as active
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::GetContext()->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pQuadVertBuffer, &stride, &offset);

	Graphics::GetContext()->m_pDeviceContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Graphics::GetContext()->m_pDeviceContext->VSSetConstantBuffers(0, 1, &(m_pQuadWVPBuffer));

	// Set Shaders to active
	Graphics::GetContext()->m_pDeviceContext->VSSetShader(m_fontShader.m_pVertexShader, 0, 0);
	Graphics::GetContext()->m_pDeviceContext->PSSetShader(m_fontShader.m_pPixelShader, 0, 0);
	Graphics::GetContext()->m_pDeviceContext->GSSetShader(m_fontShader.m_pGeometryShader, 0, 0);

	Graphics::GetContext()->m_pDeviceContext->IASetInputLayout(m_fontShader.m_pVertLayout);

	Graphics::GetContext()->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	float textWidth = 0.0f;
	for (char& c : text)
	{
		Character ch = m_characters[c];
		textWidth += ch.m_advance;
	}

	for (char& c : text) {
		// Draw a font character
		Character ch = m_characters[c];

		Matrixf posmat = Matrixf::Translate(Vec3f(float(x + ch.m_bearing.x - textWidth*0.5f), float(y - (ch.m_size.y - ch.m_bearing.y)), 0.0f));
		Matrixf scalemat = Matrixf::Scale(Vec3f(ch.m_size.x / 10.0f, ch.m_size.y / 10.0f, 1.0f));


		Matrixf world = posmat * scalemat; // transform into world space
		Matrixf projection = Matrixf::Orthographic(0, Graphics::GetContext()->m_windowWidth / Graphics::GetContext()->m_pixelScale, 0.0f, Graphics::GetContext()->m_windowHeight / Graphics::GetContext()->m_pixelScale, 0.1f, 10.0f); // transform into screen space

		Matrixf wvp = projection * world;

		m_cbCharTransform.m_wvp = wvp;
		Graphics::GetContext()->m_pDeviceContext->UpdateSubresource(m_pQuadWVPBuffer, 0, nullptr, &m_cbCharTransform, 0, 0);

		Graphics::GetContext()->m_pDeviceContext->PSSetShaderResources(0, 1, &(m_characters[c].m_charTexture));
		Graphics::GetContext()->m_pDeviceContext->PSSetSamplers(0, 1, &m_charTextureSampler);

		float blendFactor[] = { 0.0f, 0.f, 0.0f, 0.0f };

		Graphics::GetContext()->m_pDeviceContext->OMSetBlendState(m_transparency, blendFactor, 0xffffffff);

		// do 3D rendering on the back buffer here
		Graphics::GetContext()->m_pDeviceContext->DrawIndexed(5, 0, 0);

		x += ch.m_advance;
	}
}

