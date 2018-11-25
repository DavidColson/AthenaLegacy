#pragma once

#include <vector>

#include "Renderer/RenderProxy.h"
#include "RenderFont.h"

struct SDL_Window;
struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct RenderContext
{
	SDL_Window* m_pWindow;

	IDXGISwapChain* m_pSwapChain;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11RenderTargetView* m_pBackBuffer;

	// Scene is rendered into the preprocessed frame render target, 
	// where it'll then be re-rendered for post processing
	ID3D11ShaderResourceView* m_pPreprocessedFrameResourceView;
	ID3D11SamplerState* m_frameTextureSampler;
	ID3D11RenderTargetView* m_pPreprocessedFrameView;
	ID3D11Buffer * m_pFullScreenVertBuffer{ nullptr };
	ID3D11Buffer* m_pFullScreenIndexBuffer{ nullptr };
	ID3D11VertexShader* m_pPPVertexShader;
	ID3D11PixelShader* m_pPPPixelShader;

	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_pVertLayout;

	RenderFont* m_pFontRender;

	float m_pixelScale = 2.0f;
	float m_windowWidth{ 0 };
	float m_windowHeight{ 0 };

	std::vector<RenderProxy*> m_renderProxies;
};

namespace Graphics
{
	void CreateContext(SDL_Window* pWindow, float width, float height);

	RenderContext* GetContext();

	void NewFrame();

	void RenderFrame();

	void Shutdown();

	void SubmitProxy(RenderProxy* pRenderProxy);
};