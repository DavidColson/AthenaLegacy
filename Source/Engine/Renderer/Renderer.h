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
		ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
		ID3D11Texture2D* m_pTexture2D{ nullptr };
	};

	struct Shader
	{
		ID3D11InputLayout* m_pVertLayout{ nullptr };
		ID3D11VertexShader* m_pVertexShader{ nullptr };
		ID3D11GeometryShader* m_pGeometryShader{ nullptr };
		ID3D11PixelShader* m_pPixelShader{ nullptr };
	};

	Shader LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader);
	Shader LoadShaderFromText(std::string shaderContents);

	Texture2D CreateTexture2D(int width, int height, DXGI_FORMAT format, void* data, uint bindflags);
};

struct RenderContext
{
	SDL_Window* m_pWindow;

	IDXGISwapChain* m_pSwapChain;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11RenderTargetView* m_pBackBuffer;

	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D* m_pDepthStencilBuffer;

	// Scene is rendered into the preprocessed frame render target, 
	// where it'll then be re-rendered for post processing
	Graphics::Texture2D m_preprocessedFrame;
	ID3D11SamplerState* m_frameTextureSampler;
	ID3D11RenderTargetView* m_pPreprocessedFrameView;
	ID3D11Buffer * m_pFullScreenVertBuffer{ nullptr };
	Graphics::Shader m_postProcessShader;

	Graphics::Shader m_baseShader;

	RenderFont* m_pFontRender;

	float m_pixelScale = 1.0f;
	float m_windowWidth{ 0 };
	float m_windowHeight{ 0 };

	std::vector<RenderProxy*> m_renderProxies;
};