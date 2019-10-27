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

GfxDevice::Shader GfxDevice::LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader)
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

  shader.vertexInput.AddElement("POSITION", AttributeType::float3);
  shader.vertexInput.AddElement("COLOR", AttributeType::float3);
  shader.vertexInput.AddElement("TEXCOORD", AttributeType::float2);

  hr = pCtx->pDevice->CreateInputLayout(shader.vertexInput.layout.data(), UINT(shader.vertexInput.layout.size()), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &shader.vertexInput.pLayout);


  return shader;
}

GfxDevice::Shader GfxDevice::LoadShaderFromText(std::string shaderContents, bool withTexCoords /*= true*/)
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
  hr = pCtx->pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &shader.pVertexShader);
  hr = pCtx->pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &shader.pPixelShader);

  if (withTexCoords)
  {
    // #TODO: This should be done by user and provided when creating vertex shaders, 
    // we'll create the inputLayout type when we create the vert shader
    shader.vertexInput.AddElement("POSITION", AttributeType::float3);
    shader.vertexInput.AddElement("COLOR", AttributeType::float3);
    shader.vertexInput.AddElement("TEXCOORD", AttributeType::float2);

    hr = pCtx->pDevice->CreateInputLayout(shader.vertexInput.layout.data(), UINT(shader.vertexInput.layout.size()), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &shader.vertexInput.pLayout);
  }
  else
  {
    shader.vertexInput.AddElement("POSITION", AttributeType::float3);
    shader.vertexInput.AddElement("COLOR", AttributeType::float3);

    hr = pCtx->pDevice->CreateInputLayout(shader.vertexInput.layout.data(), UINT(shader.vertexInput.layout.size()), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &shader.vertexInput.pLayout);
  }
  return shader;
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

void GfxDevice::Shader::Bind() 
{
  pCtx->pDeviceContext->VSSetShader(pVertexShader, 0, 0);
  pCtx->pDeviceContext->PSSetShader(pPixelShader, 0, 0);
  pCtx->pDeviceContext->GSSetShader(pGeometryShader, 0, 0);
  pCtx->pDeviceContext->IASetInputLayout(vertexInput.pLayout);
  pCtx->pDeviceContext->IASetPrimitiveTopology(topology);
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

void GfxDevice::VertexBuffer::Create(size_t elements, size_t _elementSize, void* data)
{
  elementSize = UINT(_elementSize);

  D3D11_BUFFER_DESC vertexBufferDesc;
  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = elementSize * UINT(elements);
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = 0;
  vertexBufferDesc.MiscFlags = 0;
  vertexBufferDesc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA vertexBufferData;
  ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
  vertexBufferData.pSysMem = data;
  pCtx->pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pBuffer);
}

void GfxDevice::VertexBuffer::CreateDynamic(size_t elements, size_t _elementSize)
{
  elementSize = UINT(_elementSize);
  isDynamic = true;

  D3D11_BUFFER_DESC vertexBufferDesc;
  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

  vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  vertexBufferDesc.ByteWidth = elementSize * UINT(elements);
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  vertexBufferDesc.MiscFlags = 0;
  vertexBufferDesc.StructureByteStride = 0;

  pCtx->pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &pBuffer);
}

void GfxDevice::VertexBuffer::UpdateDynamicData(void* data, size_t size)
{
  if (!isDynamic)
  {
    Log::Print(Log::EErr, "Attempting to update non dynamic vertex buffer");
    return;
  }

  D3D11_MAPPED_SUBRESOURCE vertResource;
  ZeroMemory(&vertResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
  pCtx->pDeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertResource);
  memcpy(vertResource.pData, data, size);
  pCtx->pDeviceContext->Unmap(pBuffer, 0);
}

bool GfxDevice::VertexBuffer::IsInvalid()
{
  return pBuffer == nullptr;
}


void GfxDevice::VertexBuffer::Bind()
{
  UINT offset = 0;
  pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &elementSize, &offset);
}