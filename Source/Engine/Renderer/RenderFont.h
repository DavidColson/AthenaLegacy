#pragma once

#include "Maths/Maths.h"
#include "Renderer/Renderer.h"

#include <string>
#include <vector>

struct ID3D11ShaderResourceView;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct Character
{
	ID3D11ShaderResourceView* m_charTexture{ nullptr };
	vec2i m_size{ vec2(0.0f, 0.0f) };
	vec2i m_bearing{ vec2(0.0f, 0.0f) };
	int m_advance;
};

class RenderFont
{
public:
	RenderFont(std::string fontFile, int size);

	void Draw(std::string text, int x, int y);

private:
	struct cbTransform
	{
		mat4 m_wvp;
	};
	cbTransform m_cbCharTransform;

	ID3D11Buffer * m_pQuadVertBuffer{ nullptr };
	ID3D11Buffer* m_pQuadIndexBuffer{ nullptr };

	ID3D11Buffer* m_pQuadWVPBuffer{ nullptr };

	ID3D11SamplerState* m_charTextureSampler;

	ID3D11BlendState* m_transparency;

	Graphics::Shader m_fontShader;

	std::vector<Character> m_characters;
};