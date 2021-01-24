#include "GraphicsDevice.h"

#include "Engine.h"

#include <SDL_video.h>
#include <SDL_syswm.h>
#include <D3DCompiler.h>
#include <D3DCompiler.inl>
#include <d3d11.h>
#include <d3d11_1.h>

#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "Log.h"

#define MAX_HANDLES 512

// ***********************************
// D3D11 specific resource definitions
// ***********************************

struct RenderTarget
{
	TextureHandle texture;
	ID3D11RenderTargetView *pView{nullptr};
	TextureHandle depthStencilTexture;
	ID3D11DepthStencilView *pDepthStencilView{nullptr};
};

struct IndexBuffer
{
	int nElements;
	bool isDynamic{false};
	DXGI_FORMAT format;
	ID3D11Buffer *pBuffer{nullptr};
};

struct VertexBuffer
{
	bool isDynamic{false};
	UINT elementSize{0};
	ID3D11Buffer *pBuffer{nullptr};
};

struct VertexShader
{
	ID3D11InputLayout *pInputLayout{nullptr};
	ID3D11VertexShader *pShader{nullptr};
	eastl::vector<VertexInputElement> inputLayoutDescription;
};

struct PixelShader
{
	ID3D11PixelShader *pShader{nullptr};
};

struct GeometryShader
{
	ID3D11GeometryShader *pShader{nullptr};
};

struct Program
{
	VertexShaderHandle vHandle;
	VertexShader vertShader;

	PixelShaderHandle pHandle;
	PixelShader pixelShader;

	GeometryShaderHandle gHandle;
	GeometryShader geomShader;
};

struct Sampler
{
	ID3D11SamplerState *pSampler;
};

struct Texture
{
	ID3D11ShaderResourceView *pShaderResourceView{nullptr};
	ID3D11Texture2D *pTexture{nullptr};
};

struct ConstantBuffer
{
	ID3D11Buffer *pBuffer{nullptr};
};

struct BlendState
{
	BlendingInfo info;
	ID3D11BlendState *pState;
};

struct DepthTestState
{
	DepthTestInfo info;
	ID3D11DepthStencilState *pState;
};

struct Context
{
	IDXGISwapChain *pSwapChain{nullptr};
	ID3D11Device *pDevice{nullptr};
	ID3D11DeviceContext *pDeviceContext{nullptr};
	ID3D11InfoQueue *pDebugInfoQueue{nullptr};
	ID3D11Debug *pDebug{nullptr};
	ID3D11RasterizerState* pRasterizerState{nullptr};
	ID3DUserDefinedAnnotation *pUserDefinedAnnotation{nullptr};
	RenderTargetHandle backBuffer;

	// resources

	DEFINE_RESOURCE_POOLS(RenderTargetHandle, RenderTarget)
	DEFINE_RESOURCE_POOLS(VertexBufferHandle, VertexBuffer)
	DEFINE_RESOURCE_POOLS(IndexBufferHandle, IndexBuffer)
	DEFINE_RESOURCE_POOLS(VertexShaderHandle, VertexShader)
	DEFINE_RESOURCE_POOLS(PixelShaderHandle, PixelShader)
	DEFINE_RESOURCE_POOLS(GeometryShaderHandle, GeometryShader)
	DEFINE_RESOURCE_POOLS(ProgramHandle, Program)
	DEFINE_RESOURCE_POOLS(SamplerHandle, Sampler)
	DEFINE_RESOURCE_POOLS(TextureHandle, Texture)
	DEFINE_RESOURCE_POOLS(ConstBufferHandle, ConstantBuffer)
	DEFINE_RESOURCE_POOLS(BlendStateHandle, BlendState)
	DEFINE_RESOURCE_POOLS(DepthTestStateHandle, DepthTestState)
};

namespace
{
	Context *pCtx = nullptr;
}

DEFINE_GFX_HANDLE(RenderTargetHandle)
DEFINE_GFX_HANDLE(VertexBufferHandle)
DEFINE_GFX_HANDLE(IndexBufferHandle)
DEFINE_GFX_HANDLE(VertexShaderHandle)
DEFINE_GFX_HANDLE(PixelShaderHandle)
DEFINE_GFX_HANDLE(GeometryShaderHandle)
DEFINE_GFX_HANDLE(ProgramHandle)
DEFINE_GFX_HANDLE(SamplerHandle)
DEFINE_GFX_HANDLE(TextureHandle)
DEFINE_GFX_HANDLE(ConstBufferHandle)
DEFINE_GFX_HANDLE(BlendStateHandle)
DEFINE_GFX_HANDLE(DepthTestStateHandle)

// ***********************************
// D3D11 Flag conversions
// ***********************************

static const DXGI_FORMAT indexFormatLookup[2] = 
{
	DXGI_FORMAT_R16_UINT,
	DXGI_FORMAT_R32_UINT
};

// Should probably expand this to more formats at some point
static const DXGI_FORMAT formatLookup[4] =
{
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_R8G8B8A8_UNORM
};

static const uint32_t bytesPerPixelLookup[4] =
{
	16,
	1,
	4,
	4
};

static const D3D11_BLEND_OP blendOpLookup[5] =
{
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX
};

static const D3D11_BLEND blendLookup[12] =
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_BLEND_FACTOR,
	D3D11_BLEND_INV_BLEND_FACTOR
};

void SetDebugName(ID3D11DeviceChild *child, const eastl::string &name)
{
	if (child != nullptr && !name.empty())
		child->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.c_str());
}

// ***********************************************************************

void GfxDevice::Initialize(SDL_Window *pWindow, float width, float height)
{
	pCtx = new Context();

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(pWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	int multisampleCount = 4;

	// Set up D3D contexts
	// *******************

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;								// one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // how swap chain is to be used
	scd.OutputWindow = hwnd;							// the window to be used
	scd.SampleDesc.Count = Engine::GetConfig().multiSamples;// how many multisamples
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE; // windowed/full-screen mode

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

	{
		RenderTarget renderTarget;

		// First create render target texture and shader resource view
		Texture texture;
		{
			pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&(texture.pTexture));
			SetDebugName(texture.pTexture, "[TEXTURE_2D] Back Buffer");

			D3D11_TEXTURE2D_DESC textureDesc;
			texture.pTexture->GetDesc(&textureDesc);

			renderTarget.texture = pCtx->allocTextureHandle.NewHandle();
			pCtx->poolTexture[renderTarget.texture.id] = texture;
		}

		// Then create the render target view
		pCtx->pDevice->CreateRenderTargetView(texture.pTexture, nullptr, &renderTarget.pView);
		SetDebugName(renderTarget.pView, "[RENDER_TGT] Back Buffer");

		// Need to create a depth stencil texture now
		Texture depthStencilTexture;
		{
			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));
			textureDesc.Width = UINT(width);
			textureDesc.Height = UINT(height);
			textureDesc.MipLevels = textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			textureDesc.SampleDesc.Count = Engine::GetConfig().multiSamples;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;
			pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &depthStencilTexture.pTexture);
			SetDebugName(depthStencilTexture.pTexture, "[DEPTH_STCL] Back Buffer");

			renderTarget.depthStencilTexture = pCtx->allocTextureHandle.NewHandle();
			pCtx->poolTexture[renderTarget.depthStencilTexture.id] = depthStencilTexture;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		pCtx->pDevice->CreateDepthStencilView(depthStencilTexture.pTexture, &depthStencilViewDesc, &renderTarget.pDepthStencilView);
		SetDebugName(renderTarget.pDepthStencilView, "[DEPTH_STCL_VIEW] Back Buffer");

		pCtx->backBuffer = pCtx->allocRenderTargetHandle.NewHandle();
		pCtx->poolRenderTarget[pCtx->backBuffer.id] = renderTarget;
	}

	// Create the rasterizer state
    {
        D3D11_RASTERIZER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.FrontCounterClockwise = true;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
        pCtx->pDevice->CreateRasterizerState(&desc, &pCtx->pRasterizerState);
    }

	// Setup debug
	pCtx->pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void **)&pCtx->pDebugInfoQueue);
	pCtx->pDevice->QueryInterface(__uuidof(ID3D11Debug), (void **)&pCtx->pDebug);

	// #TODO: Note that this interface is only available on windows 8 or later. Might fail on older machines
	pCtx->pDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&pCtx->pUserDefinedAnnotation);
}

// ***********************************************************************

void GfxDevice::InitImgui()
{
	ImGui_ImplDX11_Init(pCtx->pDevice, pCtx->pDeviceContext);
}

// ***********************************************************************

void GfxDevice::Destroy()
{
	pCtx->pSwapChain->Release();
	FreeRenderTarget(pCtx->backBuffer);
	pCtx->pUserDefinedAnnotation->Release();

	pCtx->pDeviceContext->Flush();
	pCtx->pDeviceContext->Release();
	pCtx->pDevice->Release();
	if (pCtx->pDebug)
	{
		pCtx->pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
	}
	pCtx->pDebug->Release();
}

// ***********************************************************************

void GfxDevice::PrintQueuedDebugMessages()
{
	uint64_t message_count = pCtx->pDebugInfoQueue->GetNumStoredMessages();

	for (uint64_t i = 0; i < message_count; i++)
	{
		size_t message_size = 0;
		pCtx->pDebugInfoQueue->GetMessage(i, nullptr, &message_size);

		// Try get rid of this malloc if possible, maybe use a single frame allocator for reusing data
		D3D11_MESSAGE *message = (D3D11_MESSAGE *)malloc(message_size);
		pCtx->pDebugInfoQueue->GetMessage(i, message, &message_size);

		Log::Info("%s", message->pDescription);

		free(message);
	}

	pCtx->pDebugInfoQueue->ClearStoredMessages();
}

void GfxDevice::ResizeBackBuffer(float width, float height)
{
	FreeRenderTarget(pCtx->backBuffer);
	pCtx->pDeviceContext->ClearState();
	pCtx->pSwapChain->ResizeBuffers(1, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);

	// Make a new back buffer render target
	{
		RenderTarget renderTarget;

		// First create render target texture and shader resource view
		Texture texture;
		{
			pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&(texture.pTexture));
			SetDebugName(texture.pTexture, "[TEXTURE_2D] Back Buffer");

			D3D11_TEXTURE2D_DESC textureDesc;
			texture.pTexture->GetDesc(&textureDesc);

			renderTarget.texture = pCtx->allocTextureHandle.NewHandle();
			pCtx->poolTexture[renderTarget.texture.id] = texture;
		}

		// Then create the render target view
		pCtx->pDevice->CreateRenderTargetView(texture.pTexture, nullptr, &renderTarget.pView);
		SetDebugName(renderTarget.pView, "[RENDER_TGT] Back Buffer");

		// Need to create a depth stencil texture now
		Texture depthStencilTexture;
		{
			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));
			textureDesc.Width = UINT(width);
			textureDesc.Height = UINT(height);
			textureDesc.MipLevels = textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			textureDesc.SampleDesc.Count = Engine::GetConfig().multiSamples;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;
			pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &depthStencilTexture.pTexture);
			SetDebugName(depthStencilTexture.pTexture, "[DEPTH_STCL] Back Buffer");

			renderTarget.depthStencilTexture = pCtx->allocTextureHandle.NewHandle();
			pCtx->poolTexture[renderTarget.depthStencilTexture.id] = depthStencilTexture;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		pCtx->pDevice->CreateDepthStencilView(depthStencilTexture.pTexture, &depthStencilViewDesc, &renderTarget.pDepthStencilView);
		SetDebugName(renderTarget.pDepthStencilView, "[DEPTH_STCL_VIEW] Back Buffer");

		pCtx->backBuffer = pCtx->allocRenderTargetHandle.NewHandle();
		pCtx->poolRenderTarget[pCtx->backBuffer.id] = renderTarget;
	}
}

// ***********************************************************************

void GfxDevice::SetViewport(float x, float y, float width, float height)
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	pCtx->pDeviceContext->RSSetViewports(1, &viewport);
}

// ***********************************************************************

void GfxDevice::ClearBackBuffer(eastl::array<float, 4> color)
{
	RenderTarget &renderTarget = pCtx->poolRenderTarget[pCtx->backBuffer.id];

	pCtx->pDeviceContext->ClearRenderTargetView(renderTarget.pView, color.data());
	pCtx->pDeviceContext->ClearDepthStencilView(renderTarget.pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pCtx->pDeviceContext->RSSetState(pCtx->pRasterizerState);
}

// ***********************************************************************

void GfxDevice::SetBackBufferActive()
{
	RenderTarget &renderTarget = pCtx->poolRenderTarget[pCtx->backBuffer.id];

	pCtx->pDeviceContext->OMSetRenderTargets(1, &renderTarget.pView, renderTarget.pDepthStencilView);
}

// ***********************************************************************

void GfxDevice::PresentBackBuffer()
{
	pCtx->pSwapChain->Present(0, 0);
}

// ***********************************************************************

TextureHandle GfxDevice::CopyAndResolveBackBuffer()
{
	ID3D11Texture2D *pBackBufferTex;
	pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBufferTex);

	Texture texture;

	D3D11_TEXTURE2D_DESC textureDesc;
	pBackBufferTex->GetDesc(&textureDesc);
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;
	pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &texture.pTexture);
	SetDebugName(texture.pTexture, "[TEXTURE_2D] Back Buffer Copy");

	// Since the backbuffer texture is a multisampled resource, we're going to resolve the multisampling into a single sampled texture (effectively doing the anti-aliasing)
	pCtx->pDeviceContext->ResolveSubresource(texture.pTexture, D3D11CalcSubresource(0, 0, 1), pBackBufferTex, D3D11CalcSubresource(0, 0, 1), textureDesc.Format);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	pCtx->pDevice->CreateShaderResourceView(texture.pTexture, &shaderResourceViewDesc, &texture.pShaderResourceView);
	SetDebugName(texture.pShaderResourceView, "[SHADER_RSRC_VIEW] Back Buffer Copy");

	pBackBufferTex->Release();
	TextureHandle handle = pCtx->allocTextureHandle.NewHandle();
	pCtx->poolTexture[handle.id] = texture;
	return handle;
}

// ***********************************************************************

void GfxDevice::ClearRenderState()
{
	pCtx->pDeviceContext->ClearState();
}

// ***********************************************************************

void GfxDevice::DrawIndexed(int indexCount, int startIndex, int startVertex)
{
	pCtx->pDeviceContext->DrawIndexed(indexCount, startIndex, startVertex);
}

// ***********************************************************************

void GfxDevice::Draw(int numVerts, int startVertex)
{
	pCtx->pDeviceContext->Draw(numVerts, startVertex);
}

// ***********************************************************************

void GfxDevice::DrawInstanced(int numVerts, int numInstances, int startVertex, int startInstance)
{
	pCtx->pDeviceContext->DrawInstanced(numVerts, numInstances, startVertex, startInstance);
}

// ***********************************************************************

void GfxDevice::DrawIndexedInstanced(int indexCountPerInstance, int numInstances, int startIndex, int startVertex, int startInstance)
{
	pCtx->pDeviceContext->DrawIndexedInstanced(indexCountPerInstance, numInstances, startIndex, startVertex, startInstance);
}

// ***********************************************************************

BlendStateHandle GfxDevice::CreateBlendState(const BlendingInfo& info)
{
	BlendState state;
	state.info = info;

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = info.enabled;
	rtbd.BlendOp = blendOpLookup[static_cast<int>(info.colorOp)];
	rtbd.BlendOpAlpha = blendOpLookup[static_cast<int>(info.alphaOp)];
	rtbd.SrcBlend = blendLookup[static_cast<int>(info.source)];
	rtbd.DestBlend = blendLookup[static_cast<int>(info.destination)];
	rtbd.SrcBlendAlpha = blendLookup[static_cast<int>(info.sourceAlpha)];
	rtbd.DestBlendAlpha = blendLookup[static_cast<int>(info.destinationAlpha)];
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	pCtx->pDevice->CreateBlendState(&blendDesc, &state.pState);
	SetDebugName(state.pState, "[BLEND STATE]");

	BlendStateHandle handle = pCtx->allocBlendStateHandle.NewHandle();
	pCtx->poolBlendState[handle.id] = state;
	return handle;
}

// ***********************************************************************

void GfxDevice::SetBlending(BlendStateHandle handle)
{
	if (!IsValid(handle))
	{
		pCtx->pDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
		return;
	}
	BlendState &state = pCtx->poolBlendState[handle.id];
	pCtx->pDeviceContext->OMSetBlendState(state.pState, state.info.blendFactor.data(), 0xffffffff);
}

// ***********************************************************************

void GfxDevice::FreeBlendState(BlendStateHandle handle)
{
	if (!IsValid(handle))
		return;
	BlendState &state = pCtx->poolBlendState[handle.id];

	if (state.pState)
		state.pState->Release();
}

// ***********************************************************************

DepthTestStateHandle GfxDevice::CreateDepthTestState(const DepthTestInfo& info)
{
	DepthTestState state;
	state.info = info;

	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));

	// Depth test parameters
	depthDesc.DepthEnable = info.depthEnabled;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	depthDesc.StencilEnable = info.stencilEnabled;
	depthDesc.StencilReadMask = 0xFF;
	depthDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	pCtx->pDevice->CreateDepthStencilState(&depthDesc, &state.pState);

	SetDebugName(state.pState, "[DEPTH STATE]");

	DepthTestStateHandle handle = pCtx->allocDepthTestStateHandle.NewHandle();
	pCtx->poolDepthTestState[handle.id] = state;
	return handle;
}
	
// ***********************************************************************

void GfxDevice::SetDepthTest(DepthTestStateHandle handle)
{
	if (!IsValid(handle))
	{
		pCtx->pDeviceContext->OMSetDepthStencilState(NULL, 1);
		return;
	}
	DepthTestState &state = pCtx->poolDepthTestState[handle.id];
	pCtx->pDeviceContext->OMSetDepthStencilState(state.pState, 1);
}

// ***********************************************************************

void GfxDevice::FreeDepthTest(DepthTestStateHandle handle)
{
	if (!IsValid(handle))
		return;
	DepthTestState &state = pCtx->poolDepthTestState[handle.id];

	if (state.pState)
		state.pState->Release();
}


// ***********************************************************************

bool ShaderCompileFromFile(const wchar_t *fileName, const char *entry, const char *target, ID3DBlob **pOutBlob)
{
	HRESULT hr;
	ID3DBlob *pErrorBlob = nullptr;
	hr = D3DCompileFromFile(fileName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, D3DCOMPILE_DEBUG, 0, pOutBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
			Log::Warn("Shader Compile Error at entry %s (%s) \n\n%s", entry, target, (char *)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			return false;
		}
	}
	return true;
}

// ***********************************************************************

bool ShaderCompile(eastl::string &fileContents, const char *entry, const char *target, ID3DBlob **pOutBlob)
{
	HRESULT hr;
	ID3DBlob *pErrorBlob = nullptr;
	hr = D3DCompile(fileContents.c_str(), fileContents.size(), NULL, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, D3DCOMPILE_DEBUG, 0, pOutBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
			Log::Warn("Shader Compile Error at entry %s (%s) \n\n%s", entry, target, (char *)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			return false;
		}
	}
	return true;
}

// ***********************************************************************

void GfxDevice::SetTopologyType(TopologyType type)
{
	D3D11_PRIMITIVE_TOPOLOGY topologyType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	switch (type)
	{
	case TopologyType::TriangleStrip:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	case TopologyType::TriangleList:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case TopologyType::LineStrip:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case TopologyType::LineList:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case TopologyType::PointList:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case TopologyType::TriangleStripAdjacency:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		break;
	case TopologyType::TriangleListAdjacency:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		break;
	case TopologyType::LineStripAdjacency:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
		break;
	case TopologyType::LineListAdjacency:
		topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		break;
	}
	pCtx->pDeviceContext->IASetPrimitiveTopology(topologyType);
}

// ***********************************************************************

TextureHandle GfxDevice::CreateTexture(int width, int height, TextureFormat format, void *data, const eastl::string &debugName)
{
	Texture texture;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = textureDesc.ArraySize = 1;
	textureDesc.Format = formatLookup[static_cast<int>(format)];
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (data == nullptr)
	{
		pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &texture.pTexture);
	}
	else
	{
		D3D11_SUBRESOURCE_DATA textureBufferData;
		ZeroMemory(&textureBufferData, sizeof(textureBufferData));
		textureBufferData.pSysMem = data;
		textureBufferData.SysMemPitch = width * bytesPerPixelLookup[static_cast<int>(format)];
		pCtx->pDevice->CreateTexture2D(&textureDesc, &textureBufferData, &texture.pTexture);
	}

	SetDebugName(texture.pTexture, "[TEXTURE_2D] " + debugName);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	pCtx->pDevice->CreateShaderResourceView(texture.pTexture, &shaderResourceViewDesc, &texture.pShaderResourceView);
	SetDebugName(texture.pShaderResourceView, "[SHADER_RSRC_VIEW] " + debugName);

	TextureHandle handle = pCtx->allocTextureHandle.NewHandle();
	pCtx->poolTexture[handle.id] = texture;
	return handle;
}

// ***********************************************************************

void GfxDevice::BindTexture(TextureHandle handle, ShaderType shader, int slot)
{
	if (!IsValid(handle))
		return;

	Texture &texture = pCtx->poolTexture[handle.id];

	switch (shader)
	{
	case ShaderType::Vertex:
		pCtx->pDeviceContext->VSSetShaderResources(slot, 1, &texture.pShaderResourceView);
		break;
	case ShaderType::Pixel:
		pCtx->pDeviceContext->PSSetShaderResources(slot, 1, &texture.pShaderResourceView);
		break;
	case ShaderType::Geometry:
		pCtx->pDeviceContext->GSSetShaderResources(slot, 1, &texture.pShaderResourceView);
		break;
	}
}

// ***********************************************************************

void GfxDevice::FreeTexture(TextureHandle handle)
{
	if (!IsValid(handle))
		return;

	Texture &texture = pCtx->poolTexture[handle.id];

	if (texture.pTexture)
		texture.pTexture->Release();
	if (texture.pShaderResourceView)
		texture.pShaderResourceView->Release();

	texture.pTexture = nullptr;
	texture.pShaderResourceView = nullptr;
	pCtx->allocTextureHandle.FreeHandle(handle);
}

// ***********************************************************************

void* GfxDevice::GetImGuiTextureID(TextureHandle handle)
{
	if (!IsValid(handle))
		return nullptr;

	Texture &texture = pCtx->poolTexture[handle.id];

	return texture.pShaderResourceView;
}

// ***********************************************************************

RenderTargetHandle GfxDevice::CreateRenderTarget(float width, float height, int multiSamples, const eastl::string &debugName)
{
	RenderTarget renderTarget;

	// First create render target texture and shader resource view
	Texture texture;
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = UINT(width);
		textureDesc.Height = UINT(height);
		textureDesc.MipLevels = textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = multiSamples;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &texture.pTexture);
		SetDebugName(texture.pTexture, "[TEXTURE_2D] " + debugName);

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = multiSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		pCtx->pDevice->CreateShaderResourceView(texture.pTexture, &shaderResourceViewDesc, &texture.pShaderResourceView);
		SetDebugName(texture.pShaderResourceView, "[SHADER_RSRC_VIEW] " + debugName);

		renderTarget.texture = pCtx->allocTextureHandle.NewHandle();
		pCtx->poolTexture[renderTarget.texture.id] = texture;
	}

	// Then create the render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetViewDesc.ViewDimension = multiSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	pCtx->pDevice->CreateRenderTargetView(texture.pTexture, &renderTargetViewDesc, &renderTarget.pView);
	SetDebugName(renderTarget.pView, "[RENDER_TGT] " + debugName);

	// Need to create a depth stencil texture now
	Texture depthStencilTexture;
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = UINT(width);
		textureDesc.Height = UINT(height);
		textureDesc.MipLevels = textureDesc.ArraySize = 1;
		textureDesc.Format = formatLookup[static_cast<int>(TextureFormat::D24S8)];
		textureDesc.SampleDesc.Count = multiSamples;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &depthStencilTexture.pTexture);
		SetDebugName(depthStencilTexture.pTexture, "[DEPTH_STCL] " + debugName);

		renderTarget.depthStencilTexture = pCtx->allocTextureHandle.NewHandle();
		pCtx->poolTexture[renderTarget.depthStencilTexture.id] = depthStencilTexture;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = multiSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	pCtx->pDevice->CreateDepthStencilView(depthStencilTexture.pTexture, &depthStencilViewDesc, &renderTarget.pDepthStencilView);
	SetDebugName(renderTarget.pDepthStencilView, "[DEPTH_STCL_VIEW] " + debugName);

	RenderTargetHandle handle = pCtx->allocRenderTargetHandle.NewHandle();
	pCtx->poolRenderTarget[handle.id] = renderTarget;
	return handle;
}

// ***********************************************************************

void GfxDevice::BindRenderTarget(RenderTargetHandle handle)
{
	if (!IsValid(handle))
		return;

	RenderTarget &renderTarget = pCtx->poolRenderTarget[handle.id];

	pCtx->pDeviceContext->OMSetRenderTargets(1, &renderTarget.pView, renderTarget.pDepthStencilView);
}

// ***********************************************************************

void GfxDevice::UnbindRenderTarget(RenderTargetHandle handle)
{
	if (!IsValid(handle))
		return;

	ID3D11RenderTargetView *nullViews[] = {nullptr};
	pCtx->pDeviceContext->OMSetRenderTargets(1, nullViews, nullptr);
}

// ***********************************************************************

void GfxDevice::ClearRenderTarget(RenderTargetHandle handle, eastl::array<float, 4> color, bool clearDepth, bool clearStencil)
{
	if (!IsValid(handle))
		return;

	RenderTarget &renderTarget = pCtx->poolRenderTarget[handle.id];

	UINT clearFlags = 0;
	if (clearDepth) clearFlags |= D3D11_CLEAR_DEPTH;
	if (clearStencil) clearFlags |= D3D11_CLEAR_STENCIL;
	pCtx->pDeviceContext->ClearRenderTargetView(renderTarget.pView, color.data());
	pCtx->pDeviceContext->ClearDepthStencilView(renderTarget.pDepthStencilView,  clearFlags, 1.0f, 0);
	pCtx->pDeviceContext->RSSetState(pCtx->pRasterizerState);
}

// ***********************************************************************

TextureHandle GfxDevice::GetTexture(RenderTargetHandle handle)
{
	if (!IsValid(handle))
		return TextureHandle();

	RenderTarget &renderTarget = pCtx->poolRenderTarget[handle.id];
	return renderTarget.texture;
}

// ***********************************************************************

TextureHandle GfxDevice::MakeResolvedTexture(RenderTargetHandle handle)
{
	if (!IsValid(handle))
		return TextureHandle();

	RenderTarget &renderTarget = pCtx->poolRenderTarget[handle.id];
	Texture renderTex = pCtx->poolTexture[renderTarget.texture.id];
	Texture resolvedTexture;

	D3D11_TEXTURE2D_DESC textureDesc;
	renderTex.pTexture->GetDesc(&textureDesc);
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;
	pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &resolvedTexture.pTexture);
	SetDebugName(resolvedTexture.pTexture, "[TEXTURE_2D] Resolved multisampled render texture (name me better)");

	pCtx->pDeviceContext->ResolveSubresource(resolvedTexture.pTexture, D3D11CalcSubresource(0, 0, 1), renderTex.pTexture, D3D11CalcSubresource(0, 0, 1), textureDesc.Format);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	pCtx->pDevice->CreateShaderResourceView(resolvedTexture.pTexture, &shaderResourceViewDesc, &resolvedTexture.pShaderResourceView);
	SetDebugName(resolvedTexture.pShaderResourceView, "[SHADER_RSRC_VIEW] Resolved multisampled render texture (name me better)");

	TextureHandle resolvedTexHandle = pCtx->allocTextureHandle.NewHandle();
	pCtx->poolTexture[resolvedTexHandle.id] = resolvedTexture;
	return resolvedTexHandle;
}

// ***********************************************************************

void GfxDevice::FreeRenderTarget(RenderTargetHandle handle)
{
	if (!IsValid(handle))
		return;

	RenderTarget &target = pCtx->poolRenderTarget[handle.id];

	FreeTexture(target.texture);
	FreeTexture(target.depthStencilTexture);
	if (target.pView)
		target.pView->Release();
	if (target.pDepthStencilView)
		target.pDepthStencilView->Release();

	pCtx->allocRenderTargetHandle.FreeHandle(handle);
}

// ***********************************************************************

IndexBufferHandle GfxDevice::CreateIndexBuffer(size_t numElements, IndexFormat format, void *data, const eastl::string &debugName)
{
	IndexBuffer indexBuffer;

	indexBuffer.nElements = (int)numElements;
	indexBuffer.format = indexFormatLookup[static_cast<int>(format)];

	UINT elementSize = sizeof(unsigned short);
	if (format == IndexFormat::UInt)
		elementSize = sizeof(unsigned int);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = elementSize * UINT(numElements);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = data;
	HRESULT hr =pCtx->pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer.pBuffer);

	SetDebugName(indexBuffer.pBuffer, "[INDEX_BUFFER] " + debugName);

	IndexBufferHandle handle = pCtx->allocIndexBufferHandle.NewHandle();
	pCtx->poolIndexBuffer[handle.id] = indexBuffer;
	return handle;
}

// ***********************************************************************

IndexBufferHandle GfxDevice::CreateDynamicIndexBuffer(size_t numElements, IndexFormat format, const eastl::string &debugName)
{
	IndexBuffer indexBuffer;

	indexBuffer.isDynamic = true;
	indexBuffer.nElements = (int)numElements;
	indexBuffer.format = indexFormatLookup[static_cast<int>(format)];

	UINT elementSize = 16;
	if (format == IndexFormat::UInt)
		elementSize = 32;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	indexBufferDesc.ByteWidth = elementSize * UINT(numElements);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	pCtx->pDevice->CreateBuffer(&indexBufferDesc, nullptr, &indexBuffer.pBuffer);

	SetDebugName(indexBuffer.pBuffer, "[INDEX_BUFFER] " + debugName);

	IndexBufferHandle handle = pCtx->allocIndexBufferHandle.NewHandle();
	pCtx->poolIndexBuffer[handle.id] = indexBuffer;
	return handle;
}

// ***********************************************************************

void GfxDevice::UpdateDynamicIndexBuffer(IndexBufferHandle handle, void *data, size_t dataSize)
{
	if (!IsValid(handle))
		return;
	IndexBuffer &indexBuffer = pCtx->poolIndexBuffer[handle.id];

	if (!indexBuffer.isDynamic)
	{
		Log::Warn("Attempting to update non dynamic index buffer");
		return;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->pDeviceContext->Map(indexBuffer.pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, dataSize);
	pCtx->pDeviceContext->Unmap(indexBuffer.pBuffer, 0);
}

// ***********************************************************************

int GfxDevice::GetIndexBufferSize(IndexBufferHandle handle)
{
	if (!IsValid(handle))
		return -1;
	IndexBuffer &indexBuffer = pCtx->poolIndexBuffer[handle.id];
	return indexBuffer.nElements;
}

// ***********************************************************************

void GfxDevice::BindIndexBuffer(IndexBufferHandle handle)
{
	if (!IsValid(handle))
		return;
	IndexBuffer &indexBuffer = pCtx->poolIndexBuffer[handle.id];

	pCtx->pDeviceContext->IASetIndexBuffer(indexBuffer.pBuffer, indexBuffer.format, 0);
}

// ***********************************************************************

void GfxDevice::FreeIndexBuffer(IndexBufferHandle handle)
{
	if (!IsValid(handle))
		return;

	IndexBuffer &buffer = pCtx->poolIndexBuffer[handle.id];

	if (buffer.pBuffer)
		buffer.pBuffer->Release();
	pCtx->allocIndexBufferHandle.FreeHandle(handle);
}

// ***********************************************************************

eastl::vector<D3D11_INPUT_ELEMENT_DESC> CreateD3D11InputLayout(const eastl::vector<VertexInputElement> &layout)
{
	eastl::vector<D3D11_INPUT_ELEMENT_DESC> d3d11Layout;

	for (size_t i = 0; i < layout.size(); i++)
	{
		const VertexInputElement &elem = layout[i];

		switch (elem.type)
		{
		case AttributeType::Float4:
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex, DXGI_FORMAT_R32G32B32A32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			break;
		case AttributeType::Float3:
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex, DXGI_FORMAT_R32G32B32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			break;
		case AttributeType::Float2:
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex, DXGI_FORMAT_R32G32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			break;
		case AttributeType::Float:
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex, DXGI_FORMAT_R32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			break;
		case AttributeType::Float4x4:
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex, DXGI_FORMAT_R32G32B32A32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex + 1, DXGI_FORMAT_R32G32B32A32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex + 2, DXGI_FORMAT_R32G32B32A32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			d3d11Layout.push_back({elem.semanticName, elem.semanticIndex + 3, DXGI_FORMAT_R32G32B32A32_FLOAT, elem.slot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0});
			break;
		default:
			break;
		}
	}
	return d3d11Layout;
}

// ***********************************************************************

eastl::vector<VertexInputElement> ReflectInputLayout(ID3DBlob* pShaderBlob)
{
	eastl::vector<VertexInputElement> layout;

	// Reflect the input layout and construct it on the fly
	ID3D11ShaderReflection* pReflection = NULL;
	D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflection);

	D3D11_SHADER_DESC desc;
	pReflection->GetDesc(&desc);
	
	eastl::array<eastl::string, 2> ignoredSemanticNames = { "SV_InstanceID", "SV_VertexID" };

	for ( unsigned int i=0; i< desc.InputParameters; ++i )
    {
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;      
        pReflection->GetInputParameterDesc(i, &paramDesc);

		// Ignore included semantics such as SV_InstanceID/SV_VertexID etc
		if (eastl::find(ignoredSemanticNames.begin(), ignoredSemanticNames.end(), paramDesc.SemanticName) != ignoredSemanticNames.end())
			continue;
		
		// We force position to be float3 rather than default float4
		if (strcmp(paramDesc.SemanticName, "SV_POSITION") == 0 || strcmp(paramDesc.SemanticName, "POSITION") == 0 || strcmp(paramDesc.SemanticName, "SV_Position") == 0)
		{
			layout.emplace_back((const char*)paramDesc.SemanticName, paramDesc.SemanticIndex, AttributeType::Float3, i);
			continue;
		}



		uint32_t byteOffset = 0;
		if ( paramDesc.Mask == 1 )
        {
            if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) layout.emplace_back((const char*)paramDesc.SemanticName, paramDesc.SemanticIndex, AttributeType::Float, i);
            byteOffset += 4;
        }
        else if ( paramDesc.Mask <= 3 )
        {
            if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) layout.emplace_back((const char*)paramDesc.SemanticName, paramDesc.SemanticIndex, AttributeType::Float2, i);
            byteOffset += 8;
        }
        else if ( paramDesc.Mask <= 7 )
        {
            if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) layout.emplace_back((const char*)paramDesc.SemanticName, paramDesc.SemanticIndex, AttributeType::Float3, i);
            byteOffset += 12;
        }
        else if ( paramDesc.Mask <= 15 )
        {
            if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) layout.emplace_back((const char*)paramDesc.SemanticName, paramDesc.SemanticIndex, AttributeType::Float4, i);
            byteOffset += 16;
        }
	}
	return layout;
}

// ***********************************************************************

VertexShaderHandle GfxDevice::CreateVertexShader(const wchar_t *fileName, const char *entry, const eastl::string &debugName)
{
	VertexShader shader;

	ID3DBlob *pBlob = nullptr;
	ShaderCompileFromFile(fileName, entry, "vs_5_0", &pBlob);
	if (pBlob == nullptr)
		return INVALID_HANDLE;

	shader.inputLayoutDescription = ReflectInputLayout(pBlob);
	eastl::vector<D3D11_INPUT_ELEMENT_DESC> layout = CreateD3D11InputLayout(shader.inputLayoutDescription);

	pCtx->pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	pCtx->pDevice->CreateInputLayout(layout.data(), UINT(layout.size()), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shader.pInputLayout);
	SetDebugName(shader.pShader, "[SHADER_VERTEX] " + debugName);
	SetDebugName(shader.pInputLayout, "[INPUT_LAYOUT] " + debugName);
	pBlob->Release();

	VertexShaderHandle handle = pCtx->allocVertexShaderHandle.NewHandle();
	pCtx->poolVertexShader[handle.id] = shader;
	return handle;
}

// ***********************************************************************

VertexShaderHandle GfxDevice::CreateVertexShader(eastl::string &fileContents, const char *entry, const eastl::string &debugName)
{
	VertexShader shader;

	ID3DBlob *pBlob = nullptr;
	ShaderCompile(fileContents, entry, "vs_5_0", &pBlob);
	if (pBlob == nullptr)
		return INVALID_HANDLE;

	shader.inputLayoutDescription = ReflectInputLayout(pBlob);
	eastl::vector<D3D11_INPUT_ELEMENT_DESC> layout = CreateD3D11InputLayout(shader.inputLayoutDescription);

	pCtx->pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	pCtx->pDevice->CreateInputLayout(layout.data(), UINT(layout.size()), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shader.pInputLayout);
	SetDebugName(shader.pShader, "[SHADER_VERTEX] " + debugName);
	pBlob->Release();

	VertexShaderHandle handle = pCtx->allocVertexShaderHandle.NewHandle();
	pCtx->poolVertexShader[handle.id] = shader;
	return handle;
}

// ***********************************************************************

void GfxDevice::FreeVertexShader(VertexShaderHandle handle)
{
	if (!IsValid(handle))
		return;

	VertexShader &shader = pCtx->poolVertexShader[handle.id];

	if (shader.pShader)
		shader.pShader->Release();
	if (shader.pInputLayout)
		shader.pInputLayout->Release();
	pCtx->allocVertexShaderHandle.FreeHandle(handle);
}

// ***********************************************************************

PixelShaderHandle GfxDevice::CreatePixelShader(const wchar_t *fileName, const char *entry, const eastl::string &debugName)
{
	PixelShader shader;

	ID3DBlob *pBlob = nullptr;
	ShaderCompileFromFile(fileName, entry, "ps_5_0", &pBlob);
	if (pBlob == nullptr)
		return INVALID_HANDLE;

	pCtx->pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_PIXEL] " + debugName);
	pBlob->Release();

	PixelShaderHandle handle = pCtx->allocPixelShaderHandle.NewHandle();
	pCtx->poolPixelShader[handle.id] = shader;
	return handle;
}

// ***********************************************************************

PixelShaderHandle GfxDevice::CreatePixelShader(eastl::string &fileContents, const char *entry, const eastl::string &debugName)
{
	PixelShader shader;

	ID3DBlob *pBlob = nullptr;
	ShaderCompile(fileContents, entry, "ps_5_0", &pBlob);
	if (pBlob == nullptr)
		return INVALID_HANDLE;

	pCtx->pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_PIXEL] " + debugName);
	pBlob->Release();

	PixelShaderHandle handle = pCtx->allocPixelShaderHandle.NewHandle();
	pCtx->poolPixelShader[handle.id] = shader;
	return handle;
}

// ***********************************************************************

void GfxDevice::FreePixelShader(PixelShaderHandle handle)
{
	if (!IsValid(handle))
		return;

	PixelShader &shader = pCtx->poolPixelShader[handle.id];

	if (shader.pShader)
		shader.pShader->Release();
	pCtx->allocPixelShaderHandle.FreeHandle(handle);
}

// ***********************************************************************

GeometryShaderHandle GfxDevice::CreateGeometryShader(const wchar_t *fileName, const char *entry, const eastl::string &debugName)
{
	GeometryShader shader;

	ID3DBlob *pBlob = nullptr;
	ShaderCompileFromFile(fileName, entry, "gs_5_0", &pBlob);
	if (pBlob == nullptr)
		return INVALID_HANDLE;

	pCtx->pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_GEOM] " + debugName);
	pBlob->Release();

	GeometryShaderHandle handle = pCtx->allocGeometryShaderHandle.NewHandle();
	pCtx->poolGeometryShader[handle.id] = shader;
	return handle;
}

// ***********************************************************************

GeometryShaderHandle GfxDevice::CreateGeometryShader(eastl::string &fileContents, const char *entry, const eastl::string &debugName)
{
	GeometryShader shader;

	ID3DBlob *pBlob = nullptr;
	ShaderCompile(fileContents, entry, "gs_5_0", &pBlob);
	if (pBlob == nullptr)
		return INVALID_HANDLE;

	pCtx->pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_GEOM] " + debugName);
	pBlob->Release();

	GeometryShaderHandle handle = pCtx->allocGeometryShaderHandle.NewHandle();
	pCtx->poolGeometryShader[handle.id] = shader;
	return handle;
}

// ***********************************************************************

void GfxDevice::FreeGeometryShader(GeometryShaderHandle handle)
{
	if (!IsValid(handle))
		return;

	GeometryShader &shader = pCtx->poolGeometryShader[handle.id];

	if (shader.pShader)
		shader.pShader->Release();
	pCtx->allocGeometryShaderHandle.FreeHandle(handle);
}

// ***********************************************************************

ProgramHandle GfxDevice::CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader)
{
	Program program;

	if (!IsValid(vShader) || !IsValid(pShader))
		return ProgramHandle();

	program.vHandle = vShader;
	program.vertShader = pCtx->poolVertexShader[vShader.id];

	program.pHandle = pShader;
	program.pixelShader = pCtx->poolPixelShader[pShader.id];

	ProgramHandle handle = pCtx->allocProgramHandle.NewHandle();
	pCtx->poolProgram[handle.id] = program;
	return handle;
}

// ***********************************************************************

ProgramHandle GfxDevice::CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader, GeometryShaderHandle gShader)
{
	Program program;

	if (!IsValid(vShader) || !IsValid(pShader) || !IsValid(gShader))
		return ProgramHandle();

	program.vHandle = vShader;
	program.vertShader = pCtx->poolVertexShader[vShader.id];

	program.pHandle = pShader;
	program.pixelShader = pCtx->poolPixelShader[pShader.id];

	program.gHandle = gShader;
	program.geomShader = pCtx->poolGeometryShader[gShader.id];

	ProgramHandle handle = pCtx->allocProgramHandle.NewHandle();
	pCtx->poolProgram[handle.id] = program;
	return handle;
}

// ***********************************************************************

void GfxDevice::BindProgram(ProgramHandle handle)
{
	if (!IsValid(handle))
		return;
	Program &p = pCtx->poolProgram[handle.id];

	pCtx->pDeviceContext->VSSetShader(p.vertShader.pShader, 0, 0);
	pCtx->pDeviceContext->PSSetShader(p.pixelShader.pShader, 0, 0);
	pCtx->pDeviceContext->GSSetShader(p.geomShader.pShader, 0, 0);
	pCtx->pDeviceContext->IASetInputLayout(p.vertShader.pInputLayout);
}

// ***********************************************************************

void GfxDevice::FreeProgram(ProgramHandle handle, bool freeBoundShaders)
{
	if (!IsValid(handle))
		return;

	Program &program = pCtx->poolProgram[handle.id];

	if (freeBoundShaders)
	{
		FreeVertexShader(program.vHandle);
		FreePixelShader(program.pHandle);
		FreeGeometryShader(program.gHandle);
	}
	pCtx->allocProgramHandle.FreeHandle(handle);
}

// ***********************************************************************

SamplerHandle GfxDevice::CreateSampler(Filter filter, WrapMode wrapMode, const eastl::string &debugName)
{
	Sampler sampler;

	D3D11_FILTER samplerFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	switch (filter)
	{
	case Filter::Linear:
		samplerFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case Filter::Point:
		samplerFilter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case Filter::Anisotropic:
		samplerFilter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	D3D11_TEXTURE_ADDRESS_MODE samplerAddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
	switch (wrapMode)
	{
	case WrapMode::Wrap:
		samplerAddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case WrapMode::Mirror:
		samplerAddressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	case WrapMode::Clamp:
		samplerAddressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case WrapMode::Border:
		samplerAddressMode = D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	}

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = samplerFilter;
	sampDesc.AddressU = samplerAddressMode;
	sampDesc.AddressV = samplerAddressMode;
	sampDesc.AddressW = samplerAddressMode;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pCtx->pDevice->CreateSamplerState(&sampDesc, &sampler.pSampler);
	SetDebugName(sampler.pSampler, "[SAMPLER] " + debugName);

	SamplerHandle handle = pCtx->allocSamplerHandle.NewHandle();
	pCtx->poolSampler[handle.id] = sampler;
	return handle;
}

// ***********************************************************************

void GfxDevice::BindSampler(SamplerHandle handle, ShaderType shader, int slot)
{
	if (!IsValid(handle))
		return;
	Sampler &sampler = pCtx->poolSampler[handle.id];

	switch (shader)
	{
	case ShaderType::Vertex:
		pCtx->pDeviceContext->VSSetSamplers(slot, 1, &sampler.pSampler);
		break;
	case ShaderType::Pixel:
		pCtx->pDeviceContext->PSSetSamplers(slot, 1, &sampler.pSampler);
		break;
	case ShaderType::Geometry:
		pCtx->pDeviceContext->GSSetSamplers(slot, 1, &sampler.pSampler);
		break;
	}
}

// ***********************************************************************

void GfxDevice::FreeSampler(SamplerHandle handle)
{
	if (!IsValid(handle))
		return;

	Sampler &sampler = pCtx->poolSampler[handle.id];

	if (sampler.pSampler)
		sampler.pSampler->Release();

	pCtx->allocSamplerHandle.FreeHandle(handle);
}

// ***********************************************************************

VertexBufferHandle GfxDevice::CreateVertexBuffer(size_t numElements, size_t _elementSize, void *data, const eastl::string &debugName)
{
	VertexBuffer vBufferData;

	vBufferData.elementSize = UINT(_elementSize);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = vBufferData.elementSize * UINT(numElements);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = data;
	pCtx->pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vBufferData.pBuffer);

	SetDebugName(vBufferData.pBuffer, "[VERT_BUFFER] " + debugName);

	VertexBufferHandle handle = pCtx->allocVertexBufferHandle.NewHandle();
	pCtx->poolVertexBuffer[handle.id] = vBufferData;
	return handle;
}

// ***********************************************************************

VertexBufferHandle GfxDevice::CreateDynamicVertexBuffer(size_t numElements, size_t _elementSize, const eastl::string &debugName)
{
	VertexBuffer vBufferData;

	vBufferData.elementSize = UINT(_elementSize);
	vBufferData.isDynamic = true;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = vBufferData.elementSize * UINT(numElements);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	pCtx->pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &vBufferData.pBuffer);
	SetDebugName(vBufferData.pBuffer, "[VERT_BUFFER] " + debugName);

	VertexBufferHandle handle = pCtx->allocVertexBufferHandle.NewHandle();
	pCtx->poolVertexBuffer[handle.id] = vBufferData;
	return handle;
}

// ***********************************************************************

void GfxDevice::UpdateDynamicVertexBuffer(VertexBufferHandle handle, void *data, size_t dataSize)
{
	if (!IsValid(handle))
		return;

	VertexBuffer &buffer = pCtx->poolVertexBuffer[handle.id];
	if (!buffer.isDynamic)
	{
		Log::Warn("Attempting to update non dynamic vertex buffer");
		return;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->pDeviceContext->Map(buffer.pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, dataSize);
	pCtx->pDeviceContext->Unmap(buffer.pBuffer, 0);
}

// ***********************************************************************

void GfxDevice::BindVertexBuffers(size_t startSlot, size_t nBuffers, const VertexBufferHandle *handles)
{
	// Improvement: good opportunity for single frame allocator, or stack allocation here
	eastl::vector<ID3D11Buffer *> pBuffers;
	pBuffers.reserve(nBuffers);
	eastl::vector<UINT> offsets;
	offsets.reserve(nBuffers);
	eastl::vector<UINT> strides;
	strides.reserve(nBuffers);

	for (int i = 0; i < nBuffers; i++)
	{
		if (!IsValid(handles[i]))
			return;
		VertexBuffer &buffer = pCtx->poolVertexBuffer[handles[i].id];

		pBuffers.push_back(buffer.pBuffer);
		offsets.push_back(0);
		strides.push_back(buffer.elementSize);
	}
	pCtx->pDeviceContext->IASetVertexBuffers((UINT)startSlot, (UINT)nBuffers, pBuffers.data(), strides.data(), offsets.data());
}

// ***********************************************************************

void GfxDevice::FreeVertexBuffer(VertexBufferHandle handle)
{
	if (!IsValid(handle))
		return;

	VertexBuffer &buffer = pCtx->poolVertexBuffer[handle.id];

	if (buffer.pBuffer)
		buffer.pBuffer->Release();
	pCtx->allocVertexBufferHandle.FreeHandle(handle);
}

// ***********************************************************************

ConstBufferHandle GfxDevice::CreateConstantBuffer(uint32_t bufferSize, const eastl::string &debugName)
{
	ConstantBuffer buffer;

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bufferSize;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	pCtx->pDevice->CreateBuffer(&bufferDesc, nullptr, &buffer.pBuffer);
	SetDebugName(buffer.pBuffer, "[CONST_BUFFER] " + debugName);

	ConstBufferHandle handle = pCtx->allocConstBufferHandle.NewHandle();
	pCtx->poolConstantBuffer[handle.id] = buffer;
	return handle;
}

// ***********************************************************************

void GfxDevice::BindConstantBuffer(ConstBufferHandle handle, const void *bufferData, ShaderType shader, int slot)
{
	if (!IsValid(handle))
		return;
	ConstantBuffer &buffer = pCtx->poolConstantBuffer[handle.id];

	pCtx->pDeviceContext->UpdateSubresource(buffer.pBuffer, 0, nullptr, bufferData, 0, 0);

	switch (shader)
	{
	case ShaderType::Vertex:
		pCtx->pDeviceContext->VSSetConstantBuffers(slot, 1, &buffer.pBuffer);
		break;
	case ShaderType::Pixel:
		pCtx->pDeviceContext->PSSetConstantBuffers(slot, 1, &buffer.pBuffer);
		break;
	case ShaderType::Geometry:
		pCtx->pDeviceContext->GSSetConstantBuffers(slot, 1, &buffer.pBuffer);
		break;
	}
}

// ***********************************************************************

void GfxDevice::FreeConstBuffer(ConstBufferHandle handle)
{
	if (!IsValid(handle))
		return;

	ConstantBuffer &buffer = pCtx->poolConstantBuffer[handle.id];

	if (buffer.pBuffer)
		buffer.pBuffer->Release();
	pCtx->allocConstBufferHandle.FreeHandle(handle);
}

// ***********************************************************************

GfxDevice::AutoEvent::AutoEvent(eastl::string label)
{
	eastl::wstring wlabel(label.length(), L' ');
    eastl::copy(label.begin(), label.end(), wlabel.begin());
	pCtx->pUserDefinedAnnotation->BeginEvent(wlabel.c_str());
}

// ***********************************************************************

GfxDevice::AutoEvent::~AutoEvent()
{
	pCtx->pUserDefinedAnnotation->EndEvent();
}

// ***********************************************************************

void GfxDevice::SetDebugMarker(eastl::string label)
{
    eastl::wstring wlabel(label.length(), L' ');
    eastl::copy(label.begin(), label.end(), wlabel.begin());
	pCtx->pUserDefinedAnnotation->SetMarker(wlabel.c_str());
}
