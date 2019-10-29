#pragma once

#include <vector>
#include <array>
#include <dxgiformat.h>
#include <d3d11.h>

#include "GraphicsDevice/GraphicsDevice.h" 
#include "Renderer/RenderProxy.h"

// #TODO: Shouldn't be accessed outside GfxDevice
struct SDL_Window;
struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11GeometryShader;
struct ID3D11InputLayout;
struct ID3D11Texture2D;

class RenderFont;
struct Scene;

namespace Renderer
{
	// Render System callbacks
	void OnGameStart(Scene& scene); // should eventually be unecessary, moved to other systems/components

	void OnFrameStart();

	void OnFrame(Scene& scene, float deltaTime);

	void OnGameEnd();
};


// *******************
// Renderer Components
// *******************

struct CDrawable
{
	RenderProxy renderProxy;
	float lineThickness{ 1.5f };

	REFLECT()
};

struct CPostProcessing
{
	// Shader constant data
	struct cbPostProcessShaderData
	{
		Vec2f resolution;
		float time{ 0.1f };
		float pad{ 0.0f };
	};
	cbPostProcessShaderData postProcessShaderData;
	struct cbBloomShaderData
	{
		Vec2f direction;
		Vec2f resolution;
	};
	cbBloomShaderData bloomShaderData;

	// Graphics system resource handles
	RenderTargetHandle blurredFrame[2];
	ProgramHandle postProcessShaderProgram;
	ProgramHandle bloomShaderProgram;
	ID3D11Buffer* pPostProcessDataBuffer;
	ID3D11Buffer* pBloomDataBuffer;

	REFLECT()
};