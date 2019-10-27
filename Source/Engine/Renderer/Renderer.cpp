
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

#include "Profiler.h"
#include "Log.h"
#include "RenderFont.h"
#include "DebugDraw.h"
#include "Scene.h"

REFLECT_BEGIN(CDrawable)
REFLECT_MEMBER(lineThickness)
REFLECT_END()

REFLECT_BEGIN(CPostProcessing)
REFLECT_END()

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
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
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

	// Create back buffer render targer
	ID3D11Texture2D *pBackBuffer;
	pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	pCtx->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->pBackBuffer);
	pBackBuffer->Release();

	// Create a fullscreen quad and shader for drawing fullscreen textures to the backbuffer
	pCtx->fullScreenTextureShader = LoadShaderFromFile(L"shaders/FullScreenTexture.hlsl", false);
	
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pCtx->pDevice->CreateSamplerState(&sampDesc, &pCtx->fullScreenTextureSampler);

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

	// Render target for the main scene
	pCtx->preProcessedFrame.Init(*pCtx, pCtx->windowWidth, pCtx->windowHeight);

	// Init debug drawing
	DebugDraw::Detail::Init();

	// Init Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplSDL2_InitForVulkan(pWindow);
	ImGui_ImplDX11_Init(pCtx->pDevice, pCtx->pDeviceContext);
	io.Fonts->AddFontFromFileTTF("Source/Engine/ThirdParty/Imgui/misc/fonts/Roboto-Medium.ttf", 13.0f);
	ImGui::StyleColorsDark();
}

void Graphics::OnGameStart(Scene& scene)
{
	// Should be eventually moved to a material type when that exists
	pCtx->baseShader = LoadShaderFromFile(L"Shaders/Shader.hlsl", true);
	pCtx->baseShader.topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

	pCtx->pFontRender = new RenderFont("Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 50);

	// *****************
	// Post processing
	// *****************

	for (EntityID ent : SceneView<CPostProcessing>(scene))
	{
		CPostProcessing* pp = scene.Get<CPostProcessing>(ent);

		for (int i = 0; i < 2; ++i)
		{
			pp->blurredFrame[i].Init(*pCtx, pCtx->windowWidth / 2.0f, pCtx->windowHeight / 2.0f);
		}

		// Create post process shader data buffer
		D3D11_BUFFER_DESC ppdBufferDesc;
		ZeroMemory(&ppdBufferDesc, sizeof(ppdBufferDesc));
		ppdBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		ppdBufferDesc.ByteWidth = sizeof(CPostProcessing::cbPostProcessShaderData);
		ppdBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ppdBufferDesc.CPUAccessFlags = 0;
		ppdBufferDesc.MiscFlags = 0;
		Graphics::GetContext()->pDevice->CreateBuffer(&ppdBufferDesc, nullptr, &pp->pPostProcessDataBuffer);

		// Create bloom shader data buffer
		D3D11_BUFFER_DESC bloomBufferDesc;
		ZeroMemory(&bloomBufferDesc, sizeof(bloomBufferDesc));
		bloomBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bloomBufferDesc.ByteWidth = sizeof(CPostProcessing::cbBloomShaderData);
		bloomBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bloomBufferDesc.CPUAccessFlags = 0;
		bloomBufferDesc.MiscFlags = 0;
		Graphics::GetContext()->pDevice->CreateBuffer(&bloomBufferDesc, nullptr, &pp->pBloomDataBuffer);

		// Compile and create post processing shaders
		pp->postProcessShader = LoadShaderFromFile(L"shaders/PostProcessing.hlsl", false);
		pp->bloomShader = LoadShaderFromFile(L"shaders/Bloom.hlsl", false);
	}
}

void Graphics::OnFrameStart()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(pCtx->pWindow);
	ImGui::NewFrame();
}

void Graphics::OnFrame(Scene& scene, float deltaTime)
{
	PROFILE();

	// RenderFonts -- Picks up text component data
	// RenderPostProcessing -- Need a scene post process component, lives on singleton?
	// RenderImgui - Editor components? Maybe something for future

	// ****************
	// Render Drawables
	// ****************
	{
		// First we draw the scene into a render target
		pCtx->preProcessedFrame.SetActive(*pCtx);
		pCtx->preProcessedFrame.ClearView(*pCtx, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
		Graphics::SetViewport(*pCtx, 0.0f, 0.0f, pCtx->windowWidth, pCtx->windowHeight);

		// Set Shaders to active
		pCtx->baseShader.Bind(*pCtx);

		for (EntityID ent : SceneView<CDrawable, CTransform>(scene))
		{
			CDrawable* pDrawable = scene.Get<CDrawable>(ent);
			CTransform* pTransform = scene.Get<CTransform>(ent);

			pDrawable->renderProxy.SetTransform(pTransform->pos, pTransform->rot, pTransform->sca);
			pDrawable->renderProxy.lineThickness = pDrawable->lineThickness;
			pDrawable->renderProxy.Draw();
		}
	}

	// *********
	// Draw Text
	// *********

	// Ideally font is just another mesh with a material to draw
	// So it would go through the normal channels, specialness is in it's material and probably
	// an earlier system that prepares the quads to render
	pCtx->pFontRender->DrawSceneText(scene);

	// **********
	// Draw Debug 
	// **********

	DebugDraw::Detail::DrawQueue();

	// ***************
	// Post Processing
	// ***************

	bool wasPostProcessing = false;
	for (EntityID ent : SceneView<CPostProcessing>(scene))
	{
		wasPostProcessing = true;
		CPostProcessing* pp = scene.Get<CPostProcessing>(ent);

		pp->blurredFrame[0].SetActive(*pCtx);
		pp->blurredFrame[0].ClearView(*pCtx, { 0.0f, 0.f, 0.f, 1.0f }, false, false);
		Graphics::SetViewport(*pCtx, 0.f, 0.f, pCtx->windowWidth / 2.0f, pCtx->windowHeight / 2.0f);

		// Bind bloom shader data
		pp->bloomShader.Bind(*pCtx);
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pCtx->pFullScreenVertBuffer, &stride, &offset);
		pCtx->pDeviceContext->PSSetSamplers(0, 1, &pCtx->fullScreenTextureSampler);

		pp->bloomShaderData.resolution = Vec2f(900, 500);
		
		// Iteratively calculate bloom
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		int blurIterations = 8;
		for (int i = 0; i < blurIterations; ++i)
		{
			// Bind data
			float radius = float(blurIterations - i - 1) * 0.04f;
			pp->bloomShaderData.direction = i % 2 == 0 ? Vec2f(radius, 0.0f) : Vec2f(0.0f, radius);
			//bloomShaderData.direction = Vec2f(0.0f, radius);
			Graphics::GetContext()->pDeviceContext->UpdateSubresource(pp->pBloomDataBuffer, 0, nullptr, &pp->bloomShaderData, 0, 0);
			Graphics::GetContext()->pDeviceContext->PSSetConstantBuffers(0, 1, &(pp->pBloomDataBuffer));

			if (i == 0)
			{
				// First iteration, bind the plain, preprocessed frame
				pCtx->pDeviceContext->PSSetShaderResources(0, 1, &pCtx->preProcessedFrame.texture.pShaderResourceView);
			}
			else
			{
				pp->blurredFrame[(i + 1) % 2].UnsetActive(*pCtx);
				pCtx->pDeviceContext->PSSetShaderResources(0, 1, &pp->blurredFrame[(i + 1) % 2].texture.pShaderResourceView);
				pp->blurredFrame[i % 2].SetActive(*pCtx);
				pp->blurredFrame[i % 2].ClearView(*pCtx, { 0.0f, 0.f, 0.f, 1.0f }, false, false);
			}

			pCtx->pDeviceContext->Draw(4, 0);
		}

		// Now we'll actually render onto the backbuffer and do our final post process stage
		Graphics::SetBackBufferActive(*pCtx);
		Graphics::ClearBackBuffer(*pCtx, { 0.0f, 0.f, 0.f, 1.0f });
		Graphics::SetViewport(*pCtx, 0, 0, pCtx->windowWidth, pCtx->windowHeight);

		pp->postProcessShader.Bind(*pCtx);
		pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pCtx->pFullScreenVertBuffer, &stride, &offset);
		pCtx->pDeviceContext->PSSetShaderResources(0, 1, &pCtx->preProcessedFrame.texture.pShaderResourceView);
		pCtx->pDeviceContext->PSSetShaderResources(1, 1, &pp->blurredFrame[1].texture.pShaderResourceView);
		pCtx->pDeviceContext->PSSetSamplers(0, 1, &pCtx->fullScreenTextureSampler);
		
		pp->postProcessShaderData.resolution = Vec2f(pCtx->windowWidth, pCtx->windowHeight);
		pp->postProcessShaderData.time = float(SDL_GetTicks()) / 1000.0f;
		Graphics::GetContext()->pDeviceContext->UpdateSubresource(pp->pPostProcessDataBuffer, 0, nullptr, &pp->postProcessShaderData, 0, 0);
		Graphics::GetContext()->pDeviceContext->PSSetConstantBuffers(0, 1, &(pp->pPostProcessDataBuffer));

		// Draw post processed frame
		pCtx->pDeviceContext->Draw(4, 0);
	}

	// If there was no post processing to do, just draw the pre-processed frame on screen
	if (!wasPostProcessing)
	{
		Graphics::SetBackBufferActive(*pCtx);
		Graphics::ClearBackBuffer(*pCtx, { 0.0f, 0.f, 0.f, 1.0f });
		Graphics::SetViewport(*pCtx, 0, 0, pCtx->windowWidth, pCtx->windowHeight);

		pCtx->fullScreenTextureShader.Bind(*pCtx);
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pCtx->pFullScreenVertBuffer, &stride, &offset);
		pCtx->pDeviceContext->PSSetShaderResources(0, 1, &pCtx->preProcessedFrame.texture.pShaderResourceView);
		pCtx->pDeviceContext->PSSetSamplers(0, 1, &pCtx->fullScreenTextureSampler);

		pCtx->pDeviceContext->Draw(4, 0);
	}

	// *******************
	// Draw Imgui Overlays
	// *******************
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// ***********************
	// Present Frame to screen
	// ***********************

	// switch the back buffer and the front buffer
	Graphics::PresentBackBuffer(*pCtx);
	Graphics::ClearRenderState(*pCtx);
}

void Graphics::OnGameEnd()
{
	// #TODO: Release things
}

void Graphics::SetViewport(RenderContext& ctx, float x, float y, float width, float height)
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	ctx.pDeviceContext->RSSetViewports(1, &viewport);
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

void Graphics::Shader::Bind(RenderContext& ctx) 
{
	ctx.pDeviceContext->VSSetShader(pVertexShader, 0, 0);
	ctx.pDeviceContext->PSSetShader(pPixelShader, 0, 0);
	ctx.pDeviceContext->GSSetShader(pGeometryShader, 0, 0);
	ctx.pDeviceContext->IASetInputLayout(pVertLayout);
	ctx.pDeviceContext->IASetPrimitiveTopology(topology);
}

void Graphics::RenderTarget::Init(RenderContext& ctx, float width, float height)
{
	texture = CreateTexture2D(
		(int)width,
		(int)height,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		nullptr,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE
	);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	ctx.pDevice->CreateRenderTargetView(texture.pTexture2D, &renderTargetViewDesc, &pView);

	depthStencilTexture = CreateTexture2D(
		(int)width,
		(int)height,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		nullptr,
		D3D11_BIND_DEPTH_STENCIL
	);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	ctx.pDevice->CreateDepthStencilView(depthStencilTexture.pTexture2D, &depthStencilViewDesc, &pDepthStencilView);
}

void Graphics::RenderTarget::SetActive(RenderContext& ctx)
{
	ctx.pDeviceContext->OMSetRenderTargets(1, &pView, pDepthStencilView);
}

void Graphics::RenderTarget::UnsetActive(RenderContext& ctx)
{
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	ctx.pDeviceContext->OMSetRenderTargets(1, nullViews, nullptr);
}

void Graphics::RenderTarget::ClearView(RenderContext& ctx, std::array<float, 4> color, bool clearDepth, bool clearStencil)
{
	ctx.pDeviceContext->ClearRenderTargetView(pView, color.data());
	ctx.pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Graphics::ClearBackBuffer(RenderContext& ctx, std::array<float, 4> color)
{
	ctx.pDeviceContext->ClearRenderTargetView(ctx.pBackBuffer, color.data());
}

void Graphics::SetBackBufferActive(RenderContext& ctx)
{
	ctx.pDeviceContext->OMSetRenderTargets(1, &ctx.pBackBuffer, NULL);	
}

void Graphics::PresentBackBuffer(RenderContext& ctx)
{
	ctx.pSwapChain->Present(0, 0);
}

void Graphics::ClearRenderState(RenderContext& ctx)
{
	ctx.pDeviceContext->ClearState();
}