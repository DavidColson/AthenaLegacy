
#include "renderer.h"

#include <DirectXPackedVector.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3d10.h>
#include <D3DCompiler.h>

#include "maths/maths.h"

Renderer gRenderer;

void Renderer::Initialize(void* nativeWindowHandle, float _width, float _height)
{
	HWND hwnd = (HWND)nativeWindowHandle;
	m_width = _width;
	m_height = _height;



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
		&m_swap_chain,
		&m_device,
		NULL,
		&m_device_context);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	m_device->CreateRenderTargetView(pBackBuffer, NULL, &m_back_buffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	m_device_context->OMSetRenderTargets(1, &m_back_buffer, NULL);





	// Runtime Compile shaders
	// ***********************

	HRESULT hr;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// TODO: Shaders should be considered a material, kept somewhere so objects can share materials
	hr = D3DCompileFromFile(L"shaders/Shader.hlsl", 0, 0, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	hr = D3DCompileFromFile(L"shaders/Shader.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;

	// Create shader objects
	hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
	hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

	// Set Shaders to active
	m_device_context->VSSetShader(vertexShader, 0, 0);
	m_device_context->PSSetShader(pixelShader, 0, 0);




	// Create an input layout
	// **********************

	// TODO Input layouts should be part of shader programs
	ID3D11InputLayout* vertLayout;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = m_device->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &vertLayout);

	// Set the input layout as active
	m_device_context->IASetInputLayout(vertLayout);



	// Set primitive Topology
	// **********************

	// TODO: Should be set per object
	m_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);





	// Create viewport
	// ***************

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = m_width;
	viewport.Height = m_height;

	// Set Viewport as active
	m_device_context->RSSetViewports(1, &viewport);
}


void Renderer::RenderFrame()
{
	// clear the back buffer to a deep blue
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_device_context->ClearRenderTargetView(m_back_buffer, color);

	for (RenderProxy& proxy : m_renderProxies)
	{
		proxy.Draw();
	}

	// switch the back buffer and the front buffer
	m_swap_chain->Present(0, 0);
}

void Renderer::Shutdown()
{
	// TODO: Release things
}