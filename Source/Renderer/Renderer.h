#pragma once

#include <vector>

#include "Renderer/RenderProxy.h"

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;

class Renderer
{
public:
	void Initialize(void* pNativeWindowHandle, float width, float height);

	void RenderFrame();

	void Shutdown();

	void SubmitProxy(RenderProxy* pRenderProxy);

	std::vector<RenderProxy*> m_renderProxies;

	float m_width{ 0 };
	float m_height{ 0 };

	IDXGISwapChain* m_pSwapChain;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11RenderTargetView* m_pBackBuffer;
};

extern Renderer g_Renderer;