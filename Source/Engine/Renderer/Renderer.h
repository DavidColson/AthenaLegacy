#pragma once

#include <vector>
#include <dxgiformat.h>

#include "Renderer/RenderProxy.h"

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

struct RenderContext;
class RenderFont;
struct Scene;

namespace Graphics
{
	RenderContext* GetContext();


	// New API style
	void CreateContext(SDL_Window* pWindow, float width, float height);

	void OnGameStart(Scene& scene); // should eventually be unecessary, moved to other systems/components

	void OnFrameStart();

	void OnFrame(Scene& scene, float deltaTime);

	void OnGameEnd();

	struct Texture2D
	{
		ID3D11ShaderResourceView* pShaderResourceView{ nullptr };
		ID3D11Texture2D* pTexture2D{ nullptr };
	};

	struct Shader
	{
		ID3D11InputLayout* pVertLayout{ nullptr };
		ID3D11VertexShader* pVertexShader{ nullptr };
		ID3D11GeometryShader* pGeometryShader{ nullptr };
		ID3D11PixelShader* pPixelShader{ nullptr };
	};

	Shader LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader);
	Shader LoadShaderFromText(std::string shaderContents, bool withTexCoords = true);

	Texture2D CreateTexture2D(int width, int height, DXGI_FORMAT format, void* data, unsigned int bindflags);
};

struct RenderContext
{
	SDL_Window* pWindow;

	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11RenderTargetView* pBackBuffer;
	ID3D11DepthStencilView* pDepthStencilView;
	ID3D11Texture2D* pDepthStencilBuffer;

	// We render the scene into this framebuffer to give systems an opportunity to do 
	// post processing before we render into the backbuffer
	Graphics::Texture2D preprocessedFrame;
	ID3D11RenderTargetView* pPreprocessedFrameView;
	ID3D11Buffer * pFullScreenVertBuffer{ nullptr };
	ID3D11SamplerState* fullScreenTextureSampler;
	Graphics::Shader fullScreenTextureShader; // simple shader that draws a texture onscreen

	// Will eventually be a "material" type, assigned to drawables
	Graphics::Shader baseShader;

	// Need a separate font render system, which pre processes text
	// into meshes
	RenderFont* pFontRender;

	float pixelScale = 1.0f;
	float windowWidth{ 0 };
	float windowHeight{ 0 };
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
	Graphics::Texture2D blurredFrame[2];
	ID3D11RenderTargetView* pBlurredFrameView[2];
	Graphics::Shader postProcessShader;
	Graphics::Shader bloomShader;
	ID3D11Buffer* pPostProcessDataBuffer;
	ID3D11Buffer* pBloomDataBuffer;

	REFLECT()
};