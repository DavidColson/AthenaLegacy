
#include "Renderer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <stdio.h>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>
#include <ThirdParty/Imgui/examples/imgui_impl_dx11.h>

#include "Log.h"
#include "RenderFont.h"
#include "DebugDraw.h"


namespace {
	RenderContext* pCtx = nullptr;
}

RenderContext* Graphics::GetContext()
{
	return pCtx;
}

struct cbPostProcessShaderData
{
	Vec2f resolution;
	float time{ 0.1f };
	float pad{ 0.0f };
};
cbPostProcessShaderData postProcessShaderData;

void Graphics::CreateContext(SDL_Window* pWindow, float width, float height)
{
	pCtx = new RenderContext();
	pCtx->pWindow = pWindow;

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(pWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	pCtx->windowWidth = width;
	pCtx->windowHeight = height;



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
	scd.SampleDesc.Quality = 0;                    
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&pCtx->pSwapChain,
		&pCtx->pDevice,
		NULL,
		&pCtx->pDeviceContext);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	pCtx->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->pBackBuffer);
	pBackBuffer->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.Width = UINT(width);
	depthStencilDesc.Height = UINT(height);
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	pCtx->pDevice->CreateTexture2D(&depthStencilDesc, NULL, &pCtx->pDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	pCtx->pDevice->CreateDepthStencilView(pCtx->pDepthStencilBuffer, &depthStencilViewDesc, &pCtx->pDepthStencilView);

	// RENDER TO TEXTURE
	// *****************

	// Create a texture for the pre-processed frame
	pCtx->preprocessedFrame = CreateTexture2D(
		1800,
		1000,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		nullptr,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE
	);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	pCtx->pDevice->CreateRenderTargetView(pCtx->preprocessedFrame.pTexture2D, &renderTargetViewDesc, &pCtx->pPreprocessedFrameView);


	// D3D11_FILTER_MIN_MAG_MIP_POINT

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pCtx->pDevice->CreateSamplerState(&sampDesc, &pCtx->frameTextureSampler);

	// Create a quad to render onto
	std::vector<Vertex> quadVertices = {
		Vertex(Vec3f(-1.0f, -1.0f, 0.5f)),
		Vertex(Vec3f(-1.f, 1.f, 0.5f)),
		Vertex(Vec3f(1.f, -1.f, 0.5f)),
		Vertex(Vec3f(1.f, 1.f, 0.5f))
	};
	quadVertices[0].texCoords = Vec2f(0.0f, 1.0f);
	quadVertices[1].texCoords = Vec2f(0.0f, 0.0f);
	quadVertices[2].texCoords = Vec2f(1.0f, 1.0f);
	quadVertices[3].texCoords = Vec2f(1.0f, 0.0f);

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
	Graphics::GetContext()->pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pCtx->pFullScreenVertBuffer);

	// Create post process shader data buffer
	D3D11_BUFFER_DESC ppdBufferDesc;
	ZeroMemory(&ppdBufferDesc, sizeof(ppdBufferDesc));
	ppdBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ppdBufferDesc.ByteWidth = sizeof(cbPostProcessShaderData);
	ppdBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ppdBufferDesc.CPUAccessFlags = 0;
	ppdBufferDesc.MiscFlags = 0;
	Graphics::GetContext()->pDevice->CreateBuffer(&ppdBufferDesc, nullptr, &pCtx->pPostProcessDataBuffer);

	// Compile and create post processing shaders
	pCtx->postProcessShader = LoadShaderFromFile(L"shaders/PostProcessing.hlsl", false);



	pCtx->baseShader = LoadShaderFromFile(L"Shaders/Shader.hlsl", true);

	pCtx->pFontRender = new RenderFont("Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 50);

	DebugDraw::Detail::Init();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplSDL2_InitForVulkan(pWindow);
	ImGui_ImplDX11_Init(pCtx->pDevice, pCtx->pDeviceContext);

	io.Fonts->AddFontFromFileTTF("Source/Engine/ThirdParty/Imgui/misc/fonts/Roboto-Medium.ttf", 13.0f);

	ImGui::StyleColorsDark();
}


void Graphics::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(pCtx->pWindow);
	ImGui::NewFrame();
}

void Graphics::RenderFrame()
{
	// First we draw the scene into a render target
	pCtx->pDeviceContext->OMSetRenderTargets(1, &pCtx->pPreprocessedFrameView, pCtx->pDepthStencilView);

	// clear the back buffer to black
	float color[4] = { 0.0f, 0.f, 0.f, 1.0f };
	pCtx->pDeviceContext->ClearRenderTargetView(pCtx->pPreprocessedFrameView, color);
	pCtx->pDeviceContext->ClearDepthStencilView(pCtx->pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = pCtx->windowWidth /  pCtx->pixelScale;
	viewport.Height = pCtx->windowHeight / pCtx->pixelScale;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	pCtx->pDeviceContext->RSSetViewports(1, &viewport);

	// Set Shaders to active
	pCtx->pDeviceContext->VSSetShader(pCtx->baseShader.pVertexShader, 0, 0);
	pCtx->pDeviceContext->PSSetShader(pCtx->baseShader.pPixelShader, 0, 0);
	pCtx->pDeviceContext->GSSetShader(pCtx->baseShader.pGeometryShader, 0, 0);

	pCtx->pDeviceContext->IASetInputLayout(pCtx->baseShader.pVertLayout);

	pCtx->pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);

	for (RenderProxy* proxy : pCtx->renderQueue)
	{
		proxy->Draw();
	}
	pCtx->renderQueue.clear();
	pCtx->pFontRender->DrawQueue();
	DebugDraw::Detail::DrawQueue();


	// Now we change the render target to the swap chain back buffer, render onto a quad, and then render imgui
	pCtx->pDeviceContext->OMSetRenderTargets(1, &pCtx->pBackBuffer, NULL);
	pCtx->pDeviceContext->ClearRenderTargetView(pCtx->pBackBuffer, color);

	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = pCtx->windowWidth;
	viewport.Height = pCtx->windowHeight;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	pCtx->pDeviceContext->RSSetViewports(1, &viewport);

	
	pCtx->pDeviceContext->VSSetShader(pCtx->postProcessShader.pVertexShader, 0, 0);
	pCtx->pDeviceContext->PSSetShader(pCtx->postProcessShader.pPixelShader, 0, 0);
	pCtx->pDeviceContext->GSSetShader(pCtx->postProcessShader.pGeometryShader, 0, 0);
	pCtx->pDeviceContext->IASetInputLayout(pCtx->postProcessShader.pVertLayout);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pCtx->pFullScreenVertBuffer, &stride, &offset);
	pCtx->pDeviceContext->PSSetShaderResources(0, 1, &pCtx->preprocessedFrame.pShaderResourceView);
	pCtx->pDeviceContext->PSSetSamplers(0, 1, &pCtx->frameTextureSampler);
	pCtx->pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	postProcessShaderData.resolution = Vec2f(pCtx->windowWidth, pCtx->windowHeight);
	postProcessShaderData.time = float(SDL_GetTicks()) / 1000.0f;
	Graphics::GetContext()->pDeviceContext->UpdateSubresource(pCtx->pPostProcessDataBuffer, 0, nullptr, &postProcessShaderData, 0, 0);
	Graphics::GetContext()->pDeviceContext->PSSetConstantBuffers(0, 1, &(pCtx->pPostProcessDataBuffer));

	pCtx->pDeviceContext->Draw(4, 0);


	// Draw Imgui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	// #TODO: should probably clear all render state that we set after rendering
	ID3D11ShaderResourceView* pSRV = nullptr;
	pCtx->pDeviceContext->PSSetShaderResources(0, 1, &pSRV);

	// switch the back buffer and the front buffer
	pCtx->pSwapChain->Present(0, 0);
}

void Graphics::Shutdown()
{
	// #TODO: Release things
}

void Graphics::SubmitProxy(RenderProxy* pRenderProxy)
{
	pCtx->renderQueue.push_back(pRenderProxy);
}

Graphics::Shader Graphics::LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader)
{
	HRESULT hr;
	ID3DBlob* pVsBlob = nullptr;
	ID3DBlob* pPsBlob = nullptr;
	ID3DBlob* pGsBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	// #TODO: Shaders should be considered a material, kept somewhere so objects can share materials
	hr = D3DCompileFromFile(shaderName, 0, 0, "VSMain", "vs_5_0", D3DCOMPILE_DEBUG, 0, &pVsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			Log::Print(Log::EErr, "Vertex Shader Compile Error\n\n%s", (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}
	hr = D3DCompileFromFile(shaderName, 0, 0, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0, &pPsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			Log::Print(Log::EErr, "Pixel Shader Compile Error\n\n%s", (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}

	Shader shader;

	if (hasGeometryShader)
	{
		hr = D3DCompileFromFile(shaderName, 0, 0, "GSMain", "gs_5_0", D3DCOMPILE_DEBUG, 0, &pGsBlob, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				Log::Print(Log::EErr, "Geometry Shader Compile Error\n\n%s", (char*)pErrorBlob->GetBufferPointer());
				pErrorBlob->Release();
			}
		}
		hr = pCtx->pDevice->CreateGeometryShader(pGsBlob->GetBufferPointer(), pGsBlob->GetBufferSize(), nullptr, &shader.pGeometryShader);
	}

	// Create shader objects
	hr = pCtx->pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &shader.pVertexShader);
	hr = pCtx->pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &shader.pPixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = pCtx->pDevice->CreateInputLayout(layout, numElements, pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &shader.pVertLayout);

	return shader;
}

Graphics::Shader Graphics::LoadShaderFromText(std::string shaderContents, bool withTexCoords /*= true*/)
{
	HRESULT hr;

	ID3DBlob* pVsBlob = nullptr;
	ID3DBlob* pPsBlob = nullptr;
	ID3DBlob* pGsBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	hr = D3DCompile(shaderContents.c_str(), shaderContents.size(), NULL, 0, 0, "VSMain", "vs_5_0", 0, 0, &pVsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			Log::Print(Log::EErr, "Vert Shader Compile Error: %s", (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}
	hr = D3DCompile(shaderContents.c_str(), shaderContents.size(), NULL, 0, 0, "PSMain", "ps_5_0", 0, 0, &pPsBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			Log::Print(Log::EErr, "Pixel Shader Compile Error: %s", (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
	}

	Shader shader;

	// Create shader objects
	hr = Graphics::GetContext()->pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &shader.pVertexShader);
	hr = Graphics::GetContext()->pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &shader.pPixelShader);

	if (withTexCoords)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);
		hr = Graphics::GetContext()->pDevice->CreateInputLayout(layout, numElements, pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &shader.pVertLayout);
	}
	else
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		hr = Graphics::GetContext()->pDevice->CreateInputLayout(layout, numElements, pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &shader.pVertLayout);
	}
	return shader;
}

Graphics::Texture2D Graphics::CreateTexture2D(int width, int height, DXGI_FORMAT format, void* data, unsigned int bindflags)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = bindflags;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* pTexture = nullptr;
	if (data == nullptr)
	{
		pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture);
	}
	else
	{
		D3D11_SUBRESOURCE_DATA textureBufferData;
		ZeroMemory(&textureBufferData, sizeof(textureBufferData));
		textureBufferData.pSysMem = data;
		textureBufferData.SysMemPitch = width;
		pCtx->pDevice->CreateTexture2D(&textureDesc, &textureBufferData, &pTexture);
	}
	
	//pPreprocessingFrame->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Texture 2D Preprocessed Frame"), "Texture 2D Preprocessed Frame");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView* pShaderResourceView = nullptr;
	pCtx->pDevice->CreateShaderResourceView(pTexture, &shaderResourceViewDesc, &pShaderResourceView);

	return { pShaderResourceView, pTexture };
}
