#ifndef RENDERER_
#define RENDERER_

#include <vector>

#include "maths/maths.h"
#include "renderproxy.h"

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;

class Renderer
{
public:
	void Initialize(void* nativeWindowHandle, float _width, float _height);

	void RenderFrame();

	void Shutdown();

	void SubmitProxy(RenderProxy* pRenderProxy);

	std::vector<RenderProxy*> m_renderProxies;

	float m_width{ 0 };
	float m_height{ 0 };

	IDXGISwapChain* m_swap_chain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_device_context;
	ID3D11RenderTargetView* m_back_buffer;
};

extern Renderer gRenderer;
#endif