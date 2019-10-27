#pragma once

#include <array>
#include <vector>
#include <d3d11.h>

// *************************
// Graphics Driver Interface
// *************************

struct SDL_Window;
struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11GeometryShader;
struct ID3D11InputLayout;
struct ID3D11Texture2D;

class RenderFont;
struct Context;

namespace GfxDevice
{
  // #TODO: temp, external systems should never touch the context
  Context* GetContext();

  void Initialize(SDL_Window* pWindow, float width, float height);

  void SetViewport(float x, float y, float width, float height);

  void ClearBackBuffer(std::array<float, 4> color);

  void SetBackBufferActive();

  void PresentBackBuffer();

  void ClearRenderState();

  struct Texture2D
  {
    ID3D11ShaderResourceView* pShaderResourceView{ nullptr };
    ID3D11Texture2D* pTexture2D{ nullptr };
  };

  Texture2D CreateTexture2D(int width, int height, DXGI_FORMAT format, void* data, unsigned int bindflags);

  enum class AttributeType
  {
    float3,
    float2
  };

  struct VertexInputLayout
  {
    void AddElement(const char* name, AttributeType type);

    ID3D11InputLayout* pLayout{ nullptr };
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
  };

  // This is more like a program than a shader
  // Probably rename it
  struct Shader
  {
    void Bind();

    VertexInputLayout vertexInput;
    ID3D11VertexShader* pVertexShader{ nullptr };
    ID3D11GeometryShader* pGeometryShader{ nullptr };
    ID3D11PixelShader* pPixelShader{ nullptr };

    // Should be part of the index buffer no?
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  };

  Shader LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader);
  Shader LoadShaderFromText(std::string shaderContents, bool withTexCoords = true);

  // #TODO: Resources should have private constructors, and are created using GfxDevice Create* functions
  struct RenderTarget
  {
    void Create(float width, float height);
    void SetActive();
    void UnsetActive();
    void ClearView(std::array<float, 4> color, bool clearDepth, bool clearStencil);

    GfxDevice::Texture2D texture;
    ID3D11RenderTargetView* pView{ nullptr };
    GfxDevice::Texture2D depthStencilTexture;
    ID3D11DepthStencilView* pDepthStencilView { nullptr };
  };

  struct VertexBuffer
  {
    void Create(size_t numElements, size_t _elementSize, void* data);
    void CreateDynamic(size_t numElements, size_t _elementSize);
    void UpdateDynamicData(void* data, size_t dataSize);
    bool IsInvalid();
    void Bind();

    bool isDynamic{ false };
    UINT elementSize{ 0 };
    ID3D11Buffer* pBuffer{ nullptr };
  };

  struct IndexBuffer
  {
    void Create(size_t numElements, void* data);
    void CreateDynamic(size_t numElements);
    void UpdateDynamicData(void* data, size_t dataSize);
    bool IsInvalid();
    int  GetNumElements();
    void Bind();

    int nElements;
    bool isDynamic{ false };
    ID3D11Buffer* pBuffer{ nullptr };
  };
}

// #TODO: temp, external systems should never touch the context
struct Context
{
  SDL_Window* pWindow;

  IDXGISwapChain* pSwapChain;
  ID3D11Device* pDevice;
  ID3D11DeviceContext* pDeviceContext;
  ID3D11RenderTargetView* pBackBuffer;

  // We render the scene into this framebuffer to give systems an opportunity to do 
  // post processing before we render into the backbuffer
  // #TODO: This stuff should be part of the higher level graphics system
  GfxDevice::RenderTarget preProcessedFrame;
  GfxDevice::VertexBuffer fullScreenQuad;
  ID3D11SamplerState* fullScreenTextureSampler;
  GfxDevice::Shader fullScreenTextureShader; // simple shader that draws a texture onscreen

  // Will eventually be a "material" type, assigned to drawables
  GfxDevice::Shader baseShader;

  // Need a separate font render system, which pre processes text
  // into meshes
  RenderFont* pFontRender;

  float windowWidth{ 0 };
  float windowHeight{ 0 };
};