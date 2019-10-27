#pragma once

#include <vector>
#include <array>
#include <dxgiformat.h>
#include <d3d11.h>

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

	void CreateContext(SDL_Window* pWindow, float width, float height);

	void OnGameStart(Scene& scene); // should eventually be unecessary, moved to other systems/components

	void OnFrameStart();

	void OnFrame(Scene& scene, float deltaTime);

	void OnGameEnd();


	// Graphics API
	// #TODO: Makes sense for these to be inside a render context class, that can be subclassed
	void SetViewport(RenderContext& ctx, float x, float y, float width, float height);

	void ClearBackBuffer(RenderContext& ctx, std::array<float, 4> color);

	void SetBackBufferActive(RenderContext& ctx);

	void PresentBackBuffer(RenderContext& ctx);

	void ClearRenderState(RenderContext& ctx);

	struct Texture2D
	{
		ID3D11ShaderResourceView* pShaderResourceView{ nullptr };
		ID3D11Texture2D* pTexture2D{ nullptr };
	};

	Texture2D CreateTexture2D(int width, int height, DXGI_FORMAT format, void* data, unsigned int bindflags);

	// This is more like a program than a shader
	// Probably rename it
	struct Shader
	{
		ID3D11InputLayout* pVertLayout{ nullptr };
		ID3D11VertexShader* pVertexShader{ nullptr };
		ID3D11GeometryShader* pGeometryShader{ nullptr };
		ID3D11PixelShader* pPixelShader{ nullptr };
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		void Bind(RenderContext& ctx);
	};

	Shader LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader);
	Shader LoadShaderFromText(std::string shaderContents, bool withTexCoords = true);

	struct RenderTarget
	{
		Graphics::Texture2D texture;
		ID3D11RenderTargetView* pView{ nullptr };
		Graphics::Texture2D depthStencilTexture;
		ID3D11DepthStencilView* pDepthStencilView { nullptr };

		void Init(RenderContext& ctx, float width, float height);
		void SetActive(RenderContext& ctx);
		void UnsetActive(RenderContext& ctx);
		void ClearView(RenderContext& ctx, std::array<float, 4> color, bool clearDepth, bool clearStencil);
	};
};

struct RenderContext
{
	SDL_Window* pWindow;

	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11RenderTargetView* pBackBuffer;

	// We render the scene into this framebuffer to give systems an opportunity to do 
	// post processing before we render into the backbuffer
	Graphics::RenderTarget preProcessedFrame;
	ID3D11Buffer * pFullScreenVertBuffer{ nullptr };
	ID3D11SamplerState* fullScreenTextureSampler;
	Graphics::Shader fullScreenTextureShader; // simple shader that draws a texture onscreen

	// Will eventually be a "material" type, assigned to drawables
	Graphics::Shader baseShader;

	// Need a separate font render system, which pre processes text
	// into meshes
	RenderFont* pFontRender;

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
	Graphics::RenderTarget blurredFrame[2];
	Graphics::Shader postProcessShader;
	Graphics::Shader bloomShader;
	ID3D11Buffer* pPostProcessDataBuffer;
	ID3D11Buffer* pBloomDataBuffer;

	REFLECT()
};