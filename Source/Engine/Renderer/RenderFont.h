#pragma once

#include "Maths/Matrix.h"
#include "Maths/Vec2.h"
#include "Renderer/Renderer.h"
#include "GraphicsDevice/GraphicsDevice.h"

#include <string>
#include <vector>

struct ID3D11ShaderResourceView;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct Scene;

// **********
// Components
// **********

struct CText
{
	std::string text;
	bool visible = true;

	REFLECT()
};


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

	void DrawSceneText(Scene& scene);

private:
	struct cbTransform
	{
		Matrixf wvp;
	};
	cbTransform cbCharTransform;

	// #TODO: Systems outside GfxDevice should not be acceessing directX stuff
	GfxDevice::VertexBuffer quadBuffer;
	
	ID3D11Buffer* pQuadWVPBuffer{ nullptr };

	ID3D11SamplerState* charTextureSampler;

	ID3D11BlendState* transparency;

	GfxDevice::Program fontShader;

	std::vector<Character> characters;
};