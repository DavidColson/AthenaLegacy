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

enum class TopologyType
{
  TriangleStrip,
  TriangleList,
  LineStrip,
  LineList,
  PointList,
  TriangleStripAdjacency,
  TriangleListAdjacency,
  LineStripAdjacency,
  LineListAdjacency,
};

namespace GfxDevice
{
  // #TODO: temp, external systems should never touch the context
  Context* GetContext();

  void Initialize(SDL_Window* pWindow, float width, float height);

  void SetViewport(float x, float y, float width, float height);

  void ClearBackBuffer(std::array<float, 4> color);

  void SetTopologyType(TopologyType type);

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
    bool IsInvalid() { return pLayout == nullptr; }

    ID3D11InputLayout* pLayout{ nullptr };
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
  };

  struct VertexShader
  {
    void CreateFromFilename(const wchar_t* fileName, const char* entry, const VertexInputLayout& inputLayout);
    void CreateFromFileContents(std::string& fileContents, const char* entry, const VertexInputLayout& inputLayout);
    bool IsInvalid() { return pShader == nullptr; }

    VertexInputLayout vertexInput;
    ID3D11VertexShader* pShader{ nullptr };
  };

  struct PixelShader
  {
    void CreateFromFilename(const wchar_t* fileName, const char* entry);
    void CreateFromFileContents(std::string& fileContents, const char* entry);
    bool IsInvalid() { return pShader == nullptr; }

    ID3D11PixelShader* pShader{ nullptr };
  };

  struct GeometryShader
  {
    void CreateFromFilename(const wchar_t* fileName, const char* entry);
    void CreateFromFileContents(std::string& fileContents, const char* entry);
    bool IsInvalid() { return pShader == nullptr; }

    ID3D11GeometryShader* pShader{ nullptr };
  };

  struct Program
  {
    void Create(VertexShader vShader, PixelShader pShader);
    void Create(VertexShader vShader, PixelShader pShader, GeometryShader gShader);
    bool IsInvalid() { return !vertShader.IsInvalid() && !pixelShader.IsInvalid(); }
    void Bind();

    VertexShader vertShader;
    PixelShader pixelShader;
    GeometryShader geomShader;
  };

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
    bool IsInvalid() { return pBuffer == nullptr; }
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
    bool IsInvalid() { return pBuffer == nullptr; }
    int  GetNumElements();
    void Bind();

    int nElements;
    bool isDynamic{ false };
    ID3D11Buffer* pBuffer{ nullptr };
  };

  enum class Filter
  {
    Linear,
    Point,
    Anisotropic
  };

  enum class WrapMode
  {
    Wrap,
    Mirror,
    Clamp,
    Border
  };

  enum class ShaderType
  {
    Vertex,
    Pixel,
    Geometry
  };

  struct Sampler
  {
    void Create(Filter filter = Filter::Linear, WrapMode wrapMode = WrapMode::Wrap);
    bool IsInvalid() { return pSampler == nullptr; }
    void Bind(ShaderType shaderType, int slot);

    ID3D11SamplerState* pSampler;
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
  GfxDevice::Sampler fullScreenTextureSampler;
  GfxDevice::Program fullScreenTextureProgram; // simple shader program that draws a texture onscreen

  // Will eventually be a "material" type, assigned to drawables
  GfxDevice::Program baseShaderProgram;

  // Need a separate font render system, which pre processes text
  // into meshes
  RenderFont* pFontRender;

  float windowWidth{ 0 };
  float windowHeight{ 0 };
};