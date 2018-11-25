
#include "renderer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <stdio.h>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>
#include <ThirdParty/Imgui/examples/imgui_impl_dx11.h>

#include "Maths/Maths.h"

namespace {
	RenderContext* pCtx = nullptr;
}

RenderContext* Graphics::GetContext()
{
	return pCtx;
}

void Graphics::CreateContext(SDL_Window* pWindow, float width, float height)
{
	pCtx = new RenderContext();
	pCtx->m_pWindow = pWindow;

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(pWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	pCtx->m_windowWidth = width;
	pCtx->m_windowHeight = height;



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
		&pCtx->m_pSwapChain,
		&pCtx->m_pDevice,
		NULL,
		&pCtx->m_pDeviceContext);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	pCtx->m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	pCtx->m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->m_pBackBuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	pCtx->m_pDeviceContext->OMSetRenderTargets(1, &pCtx->m_pBackBuffer, NULL);


	// RENDER TO TEXTURE
	// *****************

	// Create a texture for the pre-processed frame
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = 900;
	textureDesc.Height = 500;
	textureDesc.MipLevels = textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* pPreprocessingFrame = nullptr;
	Graphics::GetContext()->m_pDevice->CreateTexture2D(&textureDesc, nullptr, &pPreprocessingFrame);
	pPreprocessingFrame->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Texture 2D Preprocessed Frame"), "Texture 2D Preprocessed Frame");

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	pCtx->m_pDevice->CreateRenderTargetView(pPreprocessingFrame, &renderTargetViewDesc, &pCtx->m_pPreprocessedFrameView);
	pCtx->m_pDevice->CreateRenderTargetView(pPreprocessingFrame, &renderTargetViewDesc, &pCtx->m_pPreprocessedFrameView);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	pCtx->m_pDevice->CreateShaderResourceView(pPreprocessingFrame, &shaderResourceViewDesc, &pCtx->m_pPreprocessedFrameResourceView);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pCtx->m_pDevice->CreateSamplerState(&sampDesc, &pCtx->m_frameTextureSampler);

	// Create a quad to render onto
	std::vector<Vertex> quadVertices = {
		Vertex(vec3(-1.0f, -1.0f, 0.5f), color(1.0f, 0.0f, 0.0f)),
		Vertex(vec3(-1.0f, 1.0f, 0.5f), color(0.0f, 1.0f, 0.0f)),
		Vertex(vec3(1.0f, 1.0f, 0.5f), color(0.0f, 0.0f, 1.0f)),
		Vertex(vec3(1.0f, -1.0f, 0.5f), color(0.0f, 0.0f, 1.0f))
	};
	std::vector<int> quadIndices = {
		0, 1, 2,
		3, 0
	};
	quadVertices[0].m_texCoords = vec2(0.0f, 1.0f);
	quadVertices[1].m_texCoords = vec2(0.0f, 0.0f);
	quadVertices[2].m_texCoords = vec2(1.0f, 0.0f);
	quadVertices[3].m_texCoords = vec2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * UINT(quadVertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = quadVertices.data();
	Graphics::GetContext()->m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pCtx->m_pFullScreenVertBuffer);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * UINT(quadIndices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = quadIndices.data();
	Graphics::GetContext()->m_pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &pCtx->m_pFullScreenIndexBuffer);
	
	// Compile and create post processing shaders
	HRESULT hr;
	ID3DBlob* pVsBlob = nullptr;
	ID3DBlob* pPsBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(L"shaders/PostProcessing.hlsl", 0, 0, "VSMain", "vs_5_0", 0, 0, &pVsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			Log::Print(Log::EErr, "Vert Shader Compile Error: %s", (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}
	hr = D3DCompileFromFile(L"shaders/PostProcessing.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &pPsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			Log::Print(Log::EErr, "Pixel Shader Compile Error: %s", (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}

	// Create shader objects
	hr = pCtx->m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &pCtx->m_pPPVertexShader);
	hr = pCtx->m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &pCtx->m_pPPPixelShader);
	


	// Runtime Compile shaders
	// ***********************

	pVsBlob = nullptr;
	pPsBlob = nullptr;
	pErrorBlob = nullptr;

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

	// Create shader objects
	hr = pCtx->m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &pCtx->m_pVertexShader);
	hr = pCtx->m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &pCtx->m_pPixelShader);



	// Create an input layout
	// **********************

	// TODO Input layouts should be part of shader programs
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = pCtx->m_pDevice->CreateInputLayout(layout, numElements, pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &pCtx->m_pVertLayout);


	pCtx->m_pFontRender = new RenderFont("Resources/Fonts/Hyperspace/Hyperspace.otf", 30);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplSDL2_InitForVulkan(pWindow);
	ImGui_ImplDX11_Init(pCtx->m_pDevice, pCtx->m_pDeviceContext);

	io.Fonts->AddFontFromFileTTF("Source/Engine/ThirdParty/Imgui/misc/fonts/Roboto-Medium.ttf", 13.0f);

	ImGui::StyleColorsDark();
}

void Graphics::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(pCtx->m_pWindow);
	ImGui::NewFrame();
}

void Graphics::RenderFrame()
{
	// First we draw the scene into a render target
	pCtx->m_pDeviceContext->OMSetRenderTargets(1, &pCtx->m_pPreprocessedFrameView, NULL);

	// clear the back buffer to a deep blue
	float color[4] = { 0.0f, 0.f, 0.f, 1.0f };
	pCtx->m_pDeviceContext->ClearRenderTargetView(pCtx->m_pPreprocessedFrameView, color);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = pCtx->m_windowWidth /  pCtx->m_pixelScale;
	viewport.Height = pCtx->m_windowHeight / pCtx->m_pixelScale;
	pCtx->m_pDeviceContext->RSSetViewports(1, &viewport);

	// Set Shaders to active
	pCtx->m_pDeviceContext->VSSetShader(pCtx->m_pVertexShader, 0, 0);
	pCtx->m_pDeviceContext->PSSetShader(pCtx->m_pPixelShader, 0, 0);

	pCtx->m_pDeviceContext->IASetInputLayout(pCtx->m_pVertLayout);

	pCtx->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	for (RenderProxy* proxy : pCtx->m_renderProxies)
	{
		proxy->Draw();
	}
	pCtx->m_pFontRender->Draw("Asteroids", int(pCtx->m_windowWidth / pCtx->m_pixelScale * 0.5f), int(pCtx->m_windowHeight / pCtx->m_pixelScale - 33.0f));

	// Now we change the render target to the swap chain back buffer, render onto a quad, and then render imgui
	pCtx->m_pDeviceContext->OMSetRenderTargets(1, &pCtx->m_pBackBuffer, NULL);
	pCtx->m_pDeviceContext->ClearRenderTargetView(pCtx->m_pBackBuffer, color);

	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = pCtx->m_windowWidth;
	viewport.Height = pCtx->m_windowHeight;
	pCtx->m_pDeviceContext->RSSetViewports(1, &viewport);

	pCtx->m_pDeviceContext->VSSetShader(pCtx->m_pPPVertexShader, 0, 0);
	pCtx->m_pDeviceContext->PSSetShader(pCtx->m_pPPPixelShader, 0, 0);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::GetContext()->m_pDeviceContext->IASetVertexBuffers(0, 1, &pCtx->m_pFullScreenVertBuffer, &stride, &offset);
	Graphics::GetContext()->m_pDeviceContext->IASetIndexBuffer(pCtx->m_pFullScreenIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	Graphics::GetContext()->m_pDeviceContext->PSSetShaderResources(0, 1, &pCtx->m_pPreprocessedFrameResourceView);
	Graphics::GetContext()->m_pDeviceContext->PSSetSamplers(0, 1, &pCtx->m_frameTextureSampler);
	Graphics::GetContext()->m_pDeviceContext->DrawIndexed(6, 0, 0);

	// Draw Imgui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// switch the back buffer and the front buffer
	pCtx->m_pSwapChain->Present(0, 0);
}

void Graphics::Shutdown()
{
	// TODO: Release things
}

void Graphics::SubmitProxy(RenderProxy* pRenderProxy)
{
	pCtx->m_renderProxies.push_back(pRenderProxy);
}
