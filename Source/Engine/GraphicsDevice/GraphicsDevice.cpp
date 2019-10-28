#include "GraphicsDevice.h"

#include <vector>
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

// #TODO: These should not be part of GfxDevice
#include "Renderer/RenderProxy.h"
#include "Renderer/DebugDraw.h"

namespace
{
  Context* pCtx = nullptr;
}

// #TODO: This should not be
Context* GfxDevice::GetContext()
{
  return pCtx;
}

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

  // Create back buffer render targer
  ID3D11Texture2D *pBackBuffer;
  pCtx->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  pCtx->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->pBackBuffer);
  pBackBuffer->Release();

  // Create a program for drawing full screen quads to t
  VertexInputLayout layout;
  layout.AddElement("POSITION", AttributeType::float3);
  layout.AddElement("COLOR", AttributeType::float3);
  layout.AddElement("TEXCOORD", AttributeType::float2);

  VertexShader vShader;
  vShader.CreateFromFilename(L"shaders/FullScreenTexture.hlsl", "VSMain", layout);

  PixelShader pShader;
  pShader.CreateFromFilename(L"shaders/FullScreenTexture.hlsl", "PSMain");

  pCtx->fullScreenTextureProgram.Create(vShader, pShader);
  
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

  // Vertex Buffer for fullscreen quad
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
  pCtx->fullScreenQuad.Create(quadVertices.size(), sizeof(Vertex), quadVertices.data());

  // Render target for the main scene
  pCtx->preProcessedFrame.Create(pCtx->windowWidth, pCtx->windowHeight);

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

void GfxDevice::ClearBackBuffer(std::array<float, 4> color)
{
  pCtx->pDeviceContext->ClearRenderTargetView(pCtx->pBackBuffer, color.data());
}

void GfxDevice::SetBackBufferActive()
{
  pCtx->pDeviceContext->OMSetRenderTargets(1, &pCtx->pBackBuffer, NULL);  
}

void GfxDevice::PresentBackBuffer()
{
  pCtx->pSwapChain->Present(0, 0);
}

void GfxDevice::ClearRenderState()
{
  pCtx->pDeviceContext->ClearState();
}

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

GfxDevice::Texture2D GfxDevice::CreateTexture2D(int width, int height, DXGI_FORMAT format, void* data, unsigned int bindflags)
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
  
  ID3D11ShaderResourceView* pShaderResourceView = nullptr;
  if (textureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    pCtx->pDevice->CreateShaderResourceView(pTexture, &shaderResourceViewDesc, &pShaderResourceView);
  }

  return { pShaderResourceView, pTexture };
}

void GfxDevice::RenderTarget::Create(float width, float height)
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
  pCtx->pDevice->CreateRenderTargetView(texture.pTexture2D, &renderTargetViewDesc, &pView);

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
  pCtx->pDevice->CreateDepthStencilView(depthStencilTexture.pTexture2D, &depthStencilViewDesc, &pDepthStencilView);
}

void GfxDevice::RenderTarget::SetActive()
{
  pCtx->pDeviceContext->OMSetRenderTargets(1, &pView, pDepthStencilView);
}

void GfxDevice::RenderTarget::UnsetActive()
{
  ID3D11RenderTargetView* nullViews[] = { nullptr };
  pCtx->pDeviceContext->OMSetRenderTargets(1, nullViews, nullptr);
}

void GfxDevice::RenderTarget::ClearView(std::array<float, 4> color, bool clearDepth, bool clearStencil)
{
  pCtx->pDeviceContext->ClearRenderTargetView(pView, color.data());
  pCtx->pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void GfxDevice::VertexInputLayout::AddElement(const char* name, AttributeType type)
{
  switch(type)
  {
    case AttributeType::float3:
      layout.push_back( { name, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
      break;
    case AttributeType::float2:
      layout.push_back( { name, 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
      break;
    default:
      break;  
  }
}

void GfxDevice::VertexBuffer::Create(size_t numElements, size_t _elementSize, void* data)
{
  elementSize = UINT(_elementSize);

  D3D11_BUFFER_DESC vertexBufferDesc;
  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = elementSize * UINT(numElements);
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = 0;
  vertexBufferDesc.MiscFlags = 0;
  vertexBufferDesc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA vertexBufferData;
  ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
  vertexBufferData.pSysMem = data;
  pCtx->pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pBuffer);
}

void GfxDevice::VertexBuffer::CreateDynamic(size_t numElements, size_t _elementSize)
{
  elementSize = UINT(_elementSize);
  isDynamic = true;

  D3D11_BUFFER_DESC vertexBufferDesc;
  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

  vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  vertexBufferDesc.ByteWidth = elementSize * UINT(numElements);
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  vertexBufferDesc.MiscFlags = 0;
  vertexBufferDesc.StructureByteStride = 0;

  pCtx->pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &pBuffer);
}

void GfxDevice::VertexBuffer::UpdateDynamicData(void* data, size_t dataSize)
{
  if (!isDynamic)
  {
    Log::Print(Log::EErr, "Attempting to update non dynamic vertex buffer");
    return;
  }

  D3D11_MAPPED_SUBRESOURCE resource;
  ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
  pCtx->pDeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
  memcpy(resource.pData, data, dataSize);
  pCtx->pDeviceContext->Unmap(pBuffer, 0);
}

void GfxDevice::VertexBuffer::Bind()
{
  UINT offset = 0;
  pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &elementSize, &offset);
}

void GfxDevice::IndexBuffer::Create(size_t numElements, void* data)
{
  nElements = (int)numElements;

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
  pCtx->pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &pBuffer);
}

void GfxDevice::IndexBuffer::CreateDynamic(size_t numElements)
{
  isDynamic = true;
  nElements = (int)numElements;

  D3D11_BUFFER_DESC indexBufferDesc;
  ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

  indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  indexBufferDesc.ByteWidth = sizeof(int) * UINT(numElements);
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  indexBufferDesc.MiscFlags = 0;
  indexBufferDesc.StructureByteStride = 0;

  pCtx->pDevice->CreateBuffer(&indexBufferDesc, nullptr, &pBuffer);
}

void GfxDevice::IndexBuffer::UpdateDynamicData(void* data, size_t dataSize)
{
  if (!isDynamic)
  {
    Log::Print(Log::EErr, "Attempting to update non dynamic index buffer");
    return;
  }

  D3D11_MAPPED_SUBRESOURCE resource;
  ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
  pCtx->pDeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
  memcpy(resource.pData, data, dataSize);
  pCtx->pDeviceContext->Unmap(pBuffer, 0);
}

int GfxDevice::IndexBuffer::GetNumElements()
{
  return nElements;
}

void GfxDevice::IndexBuffer::Bind()
{
  pCtx->pDeviceContext->IASetIndexBuffer(pBuffer, DXGI_FORMAT_R32_UINT, 0);
}

void GfxDevice::VertexShader::CreateFromFilename(const wchar_t* fileName, const char* entry, const VertexInputLayout& inputLayout)
{
  ID3DBlob* pBlob = nullptr;
  vertexInput = inputLayout;
  ShaderCompileFromFile(fileName, entry, "vs_5_0", &pBlob);
  pCtx->pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader);
  pCtx->pDevice->CreateInputLayout(vertexInput.layout.data(), UINT(vertexInput.layout.size()), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &vertexInput.pLayout);
}

void GfxDevice::VertexShader::CreateFromFileContents(std::string& fileContents, const char* entry, const VertexInputLayout& inputLayout)
{
  ID3DBlob* pBlob = nullptr;
  vertexInput = inputLayout;
  ShaderCompile(fileContents, entry, "vs_5_0", &pBlob);
  pCtx->pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader);
  pCtx->pDevice->CreateInputLayout(vertexInput.layout.data(), UINT(vertexInput.layout.size()), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &vertexInput.pLayout);
}

void GfxDevice::PixelShader::CreateFromFilename(const wchar_t* fileName, const char* entry)
{
  ID3DBlob* pBlob = nullptr;
  ShaderCompileFromFile(fileName, entry, "ps_5_0", &pBlob);
  pCtx->pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader);
}

void GfxDevice::PixelShader::CreateFromFileContents(std::string& fileContents, const char* entry)
{
  ID3DBlob* pBlob = nullptr;
  ShaderCompile(fileContents, entry, "ps_5_0", &pBlob);
  pCtx->pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader);
}

void GfxDevice::GeometryShader::CreateFromFilename(const wchar_t* fileName, const char* entry)
{
  ID3DBlob* pBlob = nullptr;
  ShaderCompileFromFile(fileName, entry, "gs_5_0", &pBlob);
  pCtx->pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader);
}

void GfxDevice::GeometryShader::CreateFromFileContents(std::string& fileContents, const char* entry)
{
  ID3DBlob* pBlob = nullptr;
  ShaderCompile(fileContents, entry, "gs_5_0", &pBlob);
  pCtx->pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader);
}

void GfxDevice::Program::Create(VertexShader vShader, PixelShader pShader)
{
  vertShader = vShader;
  pixelShader = pShader;
}

void GfxDevice::Program::Create(VertexShader vShader, PixelShader pShader, GeometryShader gShader)
{
  vertShader = vShader;
  pixelShader = pShader;
  geomShader = gShader;
}

void GfxDevice::Program::Bind() 
{
  pCtx->pDeviceContext->VSSetShader(vertShader.pShader, 0, 0);
  pCtx->pDeviceContext->PSSetShader(pixelShader.pShader, 0, 0);
  pCtx->pDeviceContext->GSSetShader(geomShader.pShader, 0, 0);
  pCtx->pDeviceContext->IASetInputLayout(vertShader.vertexInput.pLayout);
}
