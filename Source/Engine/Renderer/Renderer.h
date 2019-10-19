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

namespace Graphics
{
	void CreateContext(SDL_Window* pWindow, float width, float height);


	RenderContext* GetContext();

	void NewFrame();

	void RenderFrame();

	void Shutdown();

	void SubmitProxy(RenderProxy* pRenderProxy);

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

	// Scene is rendered into the preprocessed frame render target, 
	// where it'll then be re-rendered for post processing
	Graphics::Texture2D preprocessedFrame;
	ID3D11SamplerState* frameTextureSampler;
	ID3D11RenderTargetView* pPreprocessedFrameView;
	ID3D11Buffer * pFullScreenVertBuffer{ nullptr };
	Graphics::Shader postProcessShader;
	ID3D11Buffer* pPostProcessDataBuffer;

	Graphics::Shader baseShader;

	RenderFont* pFontRender;

	float pixelScale = 1.0f;
	float windowWidth{ 0 };
	float windowHeight{ 0 };

	std::vector<RenderProxy*> renderQueue;
};