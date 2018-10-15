
#include "renderer.h"

#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <stdio.h>

#include "Maths/Maths.h"

Renderer g_Renderer;

void Renderer::Initialize(void* pNativeWindowHandle, float width, float height)
{
	HWND hwnd = (HWND)pNativeWindowHandle;
	m_width = width;
	m_height = height;



	// Set up D3D contexts
	// *******************

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hwnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&m_pSwapChain,
		&m_pDevice,
		NULL,
		&m_pDeviceContext);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pBackBuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	m_pDeviceContext->OMSetRenderTargets(1, &m_pBackBuffer, NULL);





	// Runtime Compile shaders
	// ***********************

	HRESULT hr;

	ID3DBlob* pVsBlob = nullptr;
	ID3DBlob* pPsBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	// TODO: Shaders should be considered a material, kept somewhere so objects can share materials
	hr = D3DCompileFromFile(L"shaders/Shader.hlsl", 0, 0, "VSMain", "vs_5_0", 0, 0, &pVsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}
	hr = D3DCompileFromFile(L"shaders/Shader.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &pPsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}

	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;

	// Create shader objects
	hr = m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &pVertexShader);
	hr = m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &pPixelShader);

	// Set Shaders to active
	m_pDeviceContext->VSSetShader(pVertexShader, 0, 0);
	m_pDeviceContext->PSSetShader(pPixelShader, 0, 0);




	// Create an input layout
	// **********************

	// TODO Input layouts should be part of shader programs
	ID3D11InputLayout* pVertLayout;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = m_pDevice->CreateInputLayout(layout, numElements, pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &pVertLayout);

	// Set the input layout as active
	m_pDeviceContext->IASetInputLayout(pVertLayout);



	// Set primitive Topology
	// **********************

	// TODO: Should be set per object
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);





	// Create viewport
	// ***************

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = m_width;
	viewport.Height = m_height;

	// Set Viewport as active
	m_pDeviceContext->RSSetViewports(1, &viewport);
}


void Renderer::RenderFrame()
{
	// clear the back buffer to a deep blue
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pBackBuffer, color);

	for (RenderProxy* proxy : m_renderProxies)
	{
		proxy->Draw();
	}

	// switch the back buffer and the front buffer
	m_pSwapChain->Present(0, 0);
}

void Renderer::Shutdown()
{
	// TODO: Release things
}

void Renderer::SubmitProxy(RenderProxy* pRenderProxy)
{
	m_renderProxies.push_back(pRenderProxy);
}
