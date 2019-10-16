#pragma once

#include "Maths/Matrix.h"
#include "Maths/Vec2.h"
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
	ID3D11ShaderResourceView* charTexture{ nullptr };
	Vec2i size{ Vec2i(0, 0) };
	Vec2i bearing{ Vec2i(0, 0) };
	int advance;
};

class RenderFont
{
public:
	RenderFont(std::string fontFile, int size);

	void DrawQueue();
	void SubmitText(const char* text, Vec2f pos);

private:
	struct cbTransform
	{
		Matrixf wvp;
	};
	cbTransform cbCharTransform;

	ID3D11Buffer * pQuadVertBuffer{ nullptr };
	ID3D11Buffer* pQuadIndexBuffer{ nullptr };

	ID3D11Buffer* pQuadWVPBuffer{ nullptr };

	ID3D11SamplerState* charTextureSampler;

	ID3D11BlendState* transparency;

	Graphics::Shader fontShader;

	std::vector<Character> characters;

	struct QueueElement
	{
		std::string text;
		Vec2f pos;
		QueueElement(const char* text, Vec2f pos) : text(text), pos(pos) {}
	};
	std::vector<QueueElement> textQueue;
};