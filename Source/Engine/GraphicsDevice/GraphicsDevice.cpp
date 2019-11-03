#include "GraphicsDevice.h"

#include <vector>
#include <SDL.h>
#include <SDL_syswm.h>
#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d10.h>
#include <stdio.h>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>
#include <ThirdParty/Imgui/examples/imgui_impl_dx11.h>

#include "Log.h"

// #TODO: These should not be part of GfxDevice
#include "Renderer/RenderProxy.h"
#include "Renderer/DebugDraw.h"


// ***********************************
// D3D11 specific resource definitions
// ***********************************

struct RenderTarget
{
	TextureHandle texture;
	ID3D11RenderTargetView* pView{ nullptr };
	TextureHandle depthStencilTexture;
	ID3D11DepthStencilView* pDepthStencilView { nullptr };
};

struct IndexBuffer
{
	int nElements;
	bool isDynamic{ false };
	ID3D11Buffer* pBuffer{ nullptr };
};

struct VertexBuffer
{
	bool isDynamic{ false };
	UINT elementSize{ 0 };
	ID3D11Buffer* pBuffer{ nullptr };
};

struct VertexShader
{
	ID3D11InputLayout* pVertLayout{ nullptr };
	ID3D11VertexShader* pShader{ nullptr };
};

struct PixelShader
{
	ID3D11PixelShader* pShader{ nullptr };
};

struct GeometryShader
{
	ID3D11GeometryShader* pShader{ nullptr };
};

struct Program
{
	VertexShader vertShader;
	PixelShader pixelShader;
	GeometryShader geomShader;
};

struct Sampler
{
	ID3D11SamplerState* pSampler;
};

struct Texture
{
	ID3D11ShaderResourceView* pShaderResourceView{ nullptr };
	ID3D11Texture2D* pTexture{ nullptr };
};

struct ConstantBuffer
{
	ID3D11Buffer* pBuffer{ nullptr };
};

struct Context
{
	SDL_Window* pWindow;

	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11InfoQueue* pDebugInfoQueue;
	ID3DUserDefinedAnnotation* pUserDefinedAnnotation;
	ID3D11RenderTargetView* pBackBuffer;

	// resources
	std::vector<RenderTarget> renderTargets;
	std::vector<VertexBuffer> vertexBuffers;
	std::vector<IndexBuffer> indexBuffers;
	std::vector<VertexShader> vertexShaders;
	std::vector<PixelShader> pixelShaders;
	std::vector<GeometryShader> geometryShaders;
	std::vector<Program> programs;
	std::vector<Sampler> samplers;
	std::vector<Texture> textures;
	std::vector<ConstantBuffer> constBuffers;

	float windowWidth{ 0 };
	float windowHeight{ 0 };
};

namespace
{
	Context* pCtx = nullptr;
}

// ***********************************
// D3D11 Flag conversions
// ***********************************

// Should probably expand this to more formats at some point
static const DXGI_FORMAT formatLookup[3] = 
{
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_D24_UNORM_S8_UINT
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

void SetDebugName(ID3D11DeviceChild* child, const std::string& name)
{
	if (child != nullptr && !name.empty())
    	child->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.c_str());
}

// ***********************************************************************

void GfxDevice::Initialize(SDL_Window* pWindow, float width, float height)
{
	pCtx = new Context();
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

	// Create back buffer render target
	ID3D11Texture2D *pBackBuffer;
	pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	pCtx->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->pBackBuffer);
	SetDebugName(pCtx->pBackBuffer, "[RENDER_TGT] Back buffer");
	SetDebugName(pBackBuffer, "[TEXTURE_2D] Back buffer");
	pBackBuffer->Release();

	// Setup debug
	pCtx->pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void **)&pCtx->pDebugInfoQueue);

	// #TODO: Note that this interface is only available on windows 8 or later. Might fail on older machines
	pCtx->pDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&pCtx->pUserDefinedAnnotation);

	// Init Imgui (ideally one day imgui uses GfxDevice, and exists outside here)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplSDL2_InitForVulkan(pWindow);
	ImGui_ImplDX11_Init(pCtx->pDevice, pCtx->pDeviceContext);
	io.Fonts->AddFontFromFileTTF("Source/Engine/ThirdParty/Imgui/misc/fonts/Roboto-Medium.ttf", 13.0f);
	ImGui::StyleColorsDark();
}

// ***********************************************************************

void GfxDevice::PrintQueuedDebugMessages()
{
	uint64_t message_count = pCtx->pDebugInfoQueue->GetNumStoredMessages();

	for(uint64_t i = 0; i < message_count; i++){
			size_t message_size = 0;
			pCtx->pDebugInfoQueue->GetMessage(i, nullptr, &message_size); 

			// Try get rid of this malloc if possible, maybe use a single frame allocator for reusing data 
			D3D11_MESSAGE* message = (D3D11_MESSAGE*) malloc(message_size); 
			pCtx->pDebugInfoQueue->GetMessage(i, message, &message_size);

			Log::Print(Log::EGraphics, "%s", message->pDescription);

			free(message);
	}

	pCtx->pDebugInfoQueue->ClearStoredMessages();
}

// ***********************************************************************

SDL_Window* GfxDevice::GetWindow()
{
	return pCtx->pWindow;
}

// ***********************************************************************

float GfxDevice::GetWindowWidth()
{
	return pCtx->windowWidth;
}

// ***********************************************************************

float GfxDevice::GetWindowHeight()
{
	return pCtx->windowHeight;
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

void GfxDevice::ClearBackBuffer(std::array<float, 4> color)
{
	pCtx->pDeviceContext->ClearRenderTargetView(pCtx->pBackBuffer, color.data());
}

// ***********************************************************************

void GfxDevice::SetBackBufferActive()
{
	pCtx->pDeviceContext->OMSetRenderTargets(1, &pCtx->pBackBuffer, NULL);  
}

// ***********************************************************************

void GfxDevice::PresentBackBuffer()
{
	pCtx->pSwapChain->Present(0, 0);
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

void GfxDevice::SetBlending(const BlendingInfo& info)
{
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

	ID3D11BlendState* blendState;
	pCtx->pDevice->CreateBlendState(&blendDesc, &blendState);
	pCtx->pDeviceContext->OMSetBlendState(blendState, info.blendFactor.data(), 0xffffffff);
}

// ***********************************************************************

bool ShaderCompileFromFile(const wchar_t* fileName, const char* entry, const char* target, ID3DBlob** pOutBlob)
{
	HRESULT hr;
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(fileName, 0, 0, entry, target, D3DCOMPILE_DEBUG, 0, pOutBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			Log::Print(Log::EErr, "Shader Compile Error at entry %s (%s) \n\n%s", entry, target, (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			return false;
		}
	}
	return true;
}

// ***********************************************************************

bool ShaderCompile(std::string& fileContents, const char* entry, const char* target, ID3DBlob** pOutBlob)
{
	HRESULT hr;
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile(fileContents.c_str(), fileContents.size(), NULL, 0, 0, entry, target, D3DCOMPILE_DEBUG, 0, pOutBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			Log::Print(Log::EErr, "Shader Compile Error at entry %s (%s) \n\n%s", entry, target, (char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			return false;
		}
	}
	return true;
}


// ***********************************************************************

void GfxDevice::SetTopologyType(TopologyType type)
{
	D3D11_PRIMITIVE_TOPOLOGY topologyType;
	switch(type)
	{
		case TopologyType::TriangleStrip: topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
		case TopologyType::TriangleList: topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		case TopologyType::LineStrip: topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
		case TopologyType::LineList: topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break;
		case TopologyType::PointList: topologyType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break;
		case TopologyType::TriangleStripAdjacency: topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ; break;
		case TopologyType::TriangleListAdjacency: topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ; break;
		case TopologyType::LineStripAdjacency: topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ; break;
		case TopologyType::LineListAdjacency: topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ; break;
	}
	pCtx->pDeviceContext->IASetPrimitiveTopology(topologyType);
}

// ***********************************************************************

TextureHandle GfxDevice::CreateTexture(int width, int height, TextureFormat format, void* data, const std::string& debugName)
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
		textureBufferData.SysMemPitch = width;
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

	pCtx->textures.push_back(texture);
	return TextureHandle{uint16_t(pCtx->textures.size()-1)};
}

// ***********************************************************************

void GfxDevice::BindTexture(TextureHandle handle, ShaderType shader, int slot)
{
	Texture& texture = pCtx->textures[handle.id];

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

RenderTargetHandle GfxDevice::CreateRenderTarget(float width, float height, const std::string& debugName)
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
		textureDesc.Format = formatLookup[static_cast<int>(TextureFormat::RGBA32F)];
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &texture.pTexture);
		SetDebugName(texture.pTexture, "[TEXTURE_2D] " + debugName);

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		pCtx->pDevice->CreateShaderResourceView(texture.pTexture, &shaderResourceViewDesc, &texture.pShaderResourceView);
		SetDebugName(texture.pShaderResourceView, "[SHADER_RSRC_VIEW] " + debugName);

		pCtx->textures.push_back(texture);
		renderTarget.texture = TextureHandle{uint16_t(pCtx->textures.size()-1)};
	}

	// Then create the render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
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
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		pCtx->pDevice->CreateTexture2D(&textureDesc, nullptr, &depthStencilTexture.pTexture);
		SetDebugName(depthStencilTexture.pTexture, "[DEPTH_STCL] " + debugName);

		pCtx->textures.push_back(depthStencilTexture);
		renderTarget.depthStencilTexture = TextureHandle{uint16_t(pCtx->textures.size()-1)};
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	pCtx->pDevice->CreateDepthStencilView(depthStencilTexture.pTexture, &depthStencilViewDesc, &renderTarget.pDepthStencilView);
	SetDebugName(renderTarget.pDepthStencilView, "[DEPTH_STCL_VIEW] " + debugName);

	pCtx->renderTargets.push_back(renderTarget);
	return RenderTargetHandle{uint16_t(pCtx->renderTargets.size()-1)};
}

// ***********************************************************************

void GfxDevice::BindRenderTarget(RenderTargetHandle handle)
{
	RenderTarget& renderTarget = pCtx->renderTargets[handle.id];

	pCtx->pDeviceContext->OMSetRenderTargets(1, &renderTarget.pView, renderTarget.pDepthStencilView);
}

// ***********************************************************************

void GfxDevice::UnbindRenderTarget(RenderTargetHandle handle)
{
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	pCtx->pDeviceContext->OMSetRenderTargets(1, nullViews, nullptr);
}

// ***********************************************************************

void GfxDevice::ClearRenderTarget(RenderTargetHandle handle, std::array<float, 4> color, bool clearDepth, bool clearStencil)
{
	RenderTarget& renderTarget = pCtx->renderTargets[handle.id];

	pCtx->pDeviceContext->ClearRenderTargetView(renderTarget.pView, color.data());
	pCtx->pDeviceContext->ClearDepthStencilView(renderTarget.pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

// ***********************************************************************

TextureHandle GfxDevice::GetTexture(RenderTargetHandle handle)
{
	RenderTarget& renderTarget = pCtx->renderTargets[handle.id];
	return renderTarget.texture;
}

// ***********************************************************************

IndexBufferHandle GfxDevice::CreateIndexBuffer(size_t numElements, void* data, const std::string& debugName)
{
	IndexBuffer indexBuffer;

	indexBuffer.nElements = (int)numElements;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(int) * UINT(numElements);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = data;
	pCtx->pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer.pBuffer);

	SetDebugName(indexBuffer.pBuffer, "[INDEX_BUFFER] " + debugName);

	pCtx->indexBuffers.push_back(indexBuffer);
	return IndexBufferHandle{uint16_t(pCtx->indexBuffers.size()-1)};
}

// ***********************************************************************

IndexBufferHandle GfxDevice::CreateDynamicIndexBuffer(size_t numElements, const std::string& debugName)
{
	IndexBuffer indexBuffer;

	indexBuffer.isDynamic = true;
	indexBuffer.nElements = (int)numElements;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	indexBufferDesc.ByteWidth = sizeof(int) * UINT(numElements);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	pCtx->pDevice->CreateBuffer(&indexBufferDesc, nullptr, &indexBuffer.pBuffer);

	SetDebugName(indexBuffer.pBuffer, "[INDEX_BUFFER] " + debugName);

	pCtx->indexBuffers.push_back(indexBuffer);
	return IndexBufferHandle{uint16_t(pCtx->indexBuffers.size()-1)};
}

// ***********************************************************************

void GfxDevice::UpdateDynamicIndexBuffer(IndexBufferHandle handle, void* data, size_t dataSize)
{
	IndexBuffer& indexBuffer = pCtx->indexBuffers[handle.id];

	if (!indexBuffer.isDynamic)
	{
		Log::Print(Log::EErr, "Attempting to update non dynamic index buffer");
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
	IndexBuffer& indexBuffer = pCtx->indexBuffers[handle.id];
	return indexBuffer.nElements;
}

// ***********************************************************************

void GfxDevice::BindIndexBuffer(IndexBufferHandle handle)
{
	IndexBuffer& indexBuffer = pCtx->indexBuffers[handle.id];
	pCtx->pDeviceContext->IASetIndexBuffer(indexBuffer.pBuffer, DXGI_FORMAT_R32_UINT, 0);
}

// ***********************************************************************

std::vector<D3D11_INPUT_ELEMENT_DESC> CreateD3D11InputLayout(const std::vector<VertexInputElement>& layout)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d11Layout;

	for (const VertexInputElement& elem : layout)
	{
		// Todo: might want to replace with a lookup
		switch(elem.type)
		{
			case AttributeType::float3:
				d3d11Layout.push_back( { elem.name, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
				break;
			case AttributeType::float2:
				d3d11Layout.push_back( { elem.name, 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
				break;
			default:
				break;  
		}
	}
	return d3d11Layout;
}

// ***********************************************************************

VertexShaderHandle GfxDevice::CreateVertexShader(const wchar_t* fileName, const char* entry, const std::vector<VertexInputElement>& inputLayout, const std::string& debugName)
{
	VertexShader shader;

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout = CreateD3D11InputLayout(inputLayout);

	ID3DBlob* pBlob = nullptr;
	ShaderCompileFromFile(fileName, entry, "vs_5_0", &pBlob);
	pCtx->pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	pCtx->pDevice->CreateInputLayout(layout.data(), UINT(layout.size()), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shader.pVertLayout);
	SetDebugName(shader.pShader, "[SHADER_VERTEX] " + debugName);
	SetDebugName(shader.pVertLayout, "[INPUT_LAYOUT] " + debugName);

	pCtx->vertexShaders.push_back(shader);
	return VertexShaderHandle{uint16_t(pCtx->vertexShaders.size()-1)};
}

// ***********************************************************************

VertexShaderHandle GfxDevice::CreateVertexShader(std::string& fileContents, const char* entry, const std::vector<VertexInputElement>& inputLayout, const std::string& debugName)
{
	VertexShader shader;

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout = CreateD3D11InputLayout(inputLayout);

	ID3DBlob* pBlob = nullptr;
	ShaderCompile(fileContents, entry, "vs_5_0", &pBlob);
	pCtx->pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	pCtx->pDevice->CreateInputLayout(layout.data(), UINT(layout.size()), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shader.pVertLayout);
	SetDebugName(shader.pShader, "[SHADER_VERTEX] " + debugName);

	pCtx->vertexShaders.push_back(shader);
	return VertexShaderHandle{uint16_t(pCtx->vertexShaders.size()-1)};
}

// ***********************************************************************

PixelShaderHandle GfxDevice::CreatePixelShader(const wchar_t* fileName, const char* entry, const std::string& debugName)
{
	PixelShader shader;

	ID3DBlob* pBlob = nullptr;
	ShaderCompileFromFile(fileName, entry, "ps_5_0", &pBlob);
	pCtx->pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_PIXEL] " + debugName);

	pCtx->pixelShaders.push_back(shader);
	return PixelShaderHandle{uint16_t(pCtx->pixelShaders.size()-1)};
}

// ***********************************************************************

PixelShaderHandle GfxDevice::CreatePixelShader(std::string& fileContents, const char* entry, const std::string& debugName)
{
	PixelShader shader;

	ID3DBlob* pBlob = nullptr;
	ShaderCompile(fileContents, entry, "ps_5_0", &pBlob);
	pCtx->pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_PIXEL] " + debugName);

	pCtx->pixelShaders.push_back(shader);
	return PixelShaderHandle{uint16_t(pCtx->pixelShaders.size()-1)};
}

// ***********************************************************************

GeometryShaderHandle GfxDevice::CreateGeometryShader(const wchar_t* fileName, const char* entry, const std::string& debugName)
{
	GeometryShader shader;

	ID3DBlob* pBlob = nullptr;
	ShaderCompileFromFile(fileName, entry, "gs_5_0", &pBlob);
	pCtx->pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_GEOM] " + debugName);

	pCtx->geometryShaders.push_back(shader);
	return GeometryShaderHandle{uint16_t(pCtx->geometryShaders.size()-1)};
}

// ***********************************************************************

GeometryShaderHandle GfxDevice::CreateGeometryShader(std::string& fileContents, const char* entry, const std::string& debugName)
{
	GeometryShader shader;

	ID3DBlob* pBlob = nullptr;
	ShaderCompile(fileContents, entry, "gs_5_0", &pBlob);
	pCtx->pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shader.pShader);
	SetDebugName(shader.pShader, "[SHADER_GEOM] " + debugName);

	pCtx->geometryShaders.push_back(shader);
	return GeometryShaderHandle{uint16_t(pCtx->geometryShaders.size()-1)};
}

// ***********************************************************************

ProgramHandle GfxDevice::CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader)
{
	Program program;

	program.vertShader = pCtx->vertexShaders[vShader.id];
	program.pixelShader = pCtx->pixelShaders[pShader.id];

	pCtx->programs.push_back(program);
	return ProgramHandle{uint16_t(pCtx->programs.size()-1)};
}

// ***********************************************************************

ProgramHandle GfxDevice::CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader, GeometryShaderHandle gShader)
{
	Program program;

	program.vertShader = pCtx->vertexShaders[vShader.id];
	program.pixelShader = pCtx->pixelShaders[pShader.id];
	program.geomShader = pCtx->geometryShaders[gShader.id];

	pCtx->programs.push_back(program);
	return ProgramHandle{uint16_t(pCtx->programs.size()-1)};
}

// ***********************************************************************

void GfxDevice::BindProgram(ProgramHandle handle) 
{
	Program& p = pCtx->programs[handle.id];

	pCtx->pDeviceContext->VSSetShader(p.vertShader.pShader, 0, 0);
	pCtx->pDeviceContext->PSSetShader(p.pixelShader.pShader, 0, 0);
	pCtx->pDeviceContext->GSSetShader(p.geomShader.pShader, 0, 0);
	pCtx->pDeviceContext->IASetInputLayout(p.vertShader.pVertLayout);
}

// ***********************************************************************

SamplerHandle GfxDevice::CreateSampler(Filter filter, WrapMode wrapMode, const std::string& debugName)
{
	Sampler sampler;

	D3D11_FILTER samplerFilter;
	switch(filter)
	{
		case Filter::Linear: samplerFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case Filter::Point: samplerFilter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		case Filter::Anisotropic: samplerFilter = D3D11_FILTER_ANISOTROPIC; break;
	}

	D3D11_TEXTURE_ADDRESS_MODE samplerAddressMode;
	switch(wrapMode)
	{
		case WrapMode::Wrap: samplerAddressMode = D3D11_TEXTURE_ADDRESS_WRAP; break;
		case WrapMode::Mirror: samplerAddressMode = D3D11_TEXTURE_ADDRESS_MIRROR; break;
		case WrapMode::Clamp: samplerAddressMode = D3D11_TEXTURE_ADDRESS_CLAMP; break;
		case WrapMode::Border: samplerAddressMode = D3D11_TEXTURE_ADDRESS_BORDER; break;
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

	pCtx->samplers.push_back(sampler);
	return SamplerHandle{uint16_t(pCtx->samplers.size()-1)};
}

// ***********************************************************************

void GfxDevice::BindSampler(SamplerHandle handle, ShaderType shader, int slot)
{
	Sampler& sampler = pCtx->samplers[handle.id];

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

VertexBufferHandle GfxDevice::CreateVertexBuffer(size_t numElements, size_t _elementSize, void* data, const std::string& debugName)
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

	pCtx->vertexBuffers.push_back(vBufferData);
	return VertexBufferHandle{uint16_t(pCtx->vertexBuffers.size()-1)};
}

// ***********************************************************************

VertexBufferHandle GfxDevice::CreateDynamicVertexBuffer(size_t numElements, size_t _elementSize, const std::string& debugName)
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

	pCtx->vertexBuffers.push_back(vBufferData);
	return VertexBufferHandle{uint16_t(pCtx->vertexBuffers.size()-1)};
}

// ***********************************************************************

void GfxDevice::UpdateDynamicVertexBuffer(VertexBufferHandle handle, void* data, size_t dataSize)
{
	VertexBuffer& buffer = pCtx->vertexBuffers[handle.id];
	if (!buffer.isDynamic)
	{
		Log::Print(Log::EErr, "Attempting to update non dynamic vertex buffer");
		return;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->pDeviceContext->Map(buffer.pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, dataSize);
	pCtx->pDeviceContext->Unmap(buffer.pBuffer, 0);
}

// ***********************************************************************

void GfxDevice::BindVertexBuffer(VertexBufferHandle handle)
{
	VertexBuffer& buffer = pCtx->vertexBuffers[handle.id];
	UINT offset = 0;
	pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &buffer.pBuffer, &buffer.elementSize, &offset);
}

// ***********************************************************************

ConstBufferHandle GfxDevice::CreateConstantBuffer(uint32_t bufferSize, const std::string& debugName)
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

	pCtx->constBuffers.push_back(buffer);
	return ConstBufferHandle{uint16_t(pCtx->constBuffers.size()-1)};
}

// ***********************************************************************

void GfxDevice::BindConstantBuffer(ConstBufferHandle handle, const void* bufferData, ShaderType shader, int slot)
{
	ConstantBuffer& buffer = pCtx->constBuffers[handle.id];

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

GfxDevice::AutoEvent::AutoEvent(std::string label)
{
	std::wstring wide(label.begin(), label.end());
	pCtx->pUserDefinedAnnotation->BeginEvent(wide.c_str());
}

// ***********************************************************************

GfxDevice::AutoEvent::~AutoEvent()
{
	pCtx->pUserDefinedAnnotation->EndEvent();
}

// ***********************************************************************

void GfxDevice::SetDebugMarker(std::string label)
{
	std::wstring wide(label.begin(), label.end());
	pCtx->pUserDefinedAnnotation->SetMarker(wide.c_str());
}
