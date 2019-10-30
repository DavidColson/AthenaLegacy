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
	TextureHandle charTexture;
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
	struct TransformData
	{
		Matrixf wvp;
	};

	// #TODO: Systems outside GfxDevice should not be acceessing directX stuff
	VertexBufferHandle quadBuffer;
	SamplerHandle charTextureSampler;
	ProgramHandle fontShaderProgram;
	
	ConstBufferHandle wvpBuffer;
	ID3D11BlendState* transparency;

	std::vector<Character> characters;
};