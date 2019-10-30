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

#define DEFINE_HANDLE(_name)                                                          \
  struct _name { uint16_t id{ UINT16_MAX }; };                                        \
  inline bool IsValid(_name _handle) { return UINT16_MAX != _handle.id; }

#define INVALID_HANDLE { UINT16_MAX }

DEFINE_HANDLE(RenderTargetHandle)
DEFINE_HANDLE(VertexBufferHandle)
DEFINE_HANDLE(IndexBufferHandle)
DEFINE_HANDLE(PixelShaderHandle)
DEFINE_HANDLE(VertexShaderHandle)
DEFINE_HANDLE(GeometryShaderHandle)
DEFINE_HANDLE(ProgramHandle)
DEFINE_HANDLE(SamplerHandle)
DEFINE_HANDLE(TextureHandle)
DEFINE_HANDLE(ConstBufferHandle)

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

enum class AttributeType
{
  float3,
  float2
};

enum class TextureFormat
{
  // Should probably fill this out, but this'll do for now
  RGBA32F,
  R8,
  D24S8
};

struct VertexInputElement
{
  const char* name;
  AttributeType type;
};

namespace GfxDevice
{
  // #TODO: temp, external systems should never touch the context
  Context* GetContext();

  void Initialize(SDL_Window* pWindow, float width, float height);

  SDL_Window* GetWindow();

  float GetWindowWidth();

  float GetWindowHeight();

  void SetViewport(float x, float y, float width, float height);

  void ClearBackBuffer(std::array<float, 4> color);

  void SetTopologyType(TopologyType type);

  void SetBackBufferActive();

  void PresentBackBuffer();

  void ClearRenderState();

  void DrawIndexed(int indexCount, int startIndex, int startVertex);

  void Draw(int numVerts, int startVertex);

  // Vertex Buffers

  VertexBufferHandle CreateVertexBuffer(size_t numElements, size_t _elementSize, void* data);

  VertexBufferHandle CreateDynamicVertexBuffer(size_t numElements, size_t _elementSize);

  void UpdateDynamicVertexBuffer(VertexBufferHandle handle, void* data, size_t dataSize);

  void BindVertexBuffer(VertexBufferHandle handle);

  // Index Buffers

  IndexBufferHandle CreateIndexBuffer(size_t numElements, void* data);

  IndexBufferHandle CreateDynamicIndexBuffer(size_t numElements);

  void UpdateDynamicIndexBuffer(IndexBufferHandle handle, void* data, size_t dataSize);

  int GetIndexBufferSize(IndexBufferHandle handle);

  void BindIndexBuffer(IndexBufferHandle handle);

  // Shaders And Programs

  VertexShaderHandle CreateVertexShader(const wchar_t* fileName, const char* entry, const std::vector<VertexInputElement>& inputLayout);

  VertexShaderHandle CreateVertexShader(std::string& fileContents, const char* entry, const std::vector<VertexInputElement>& inputLayout);

  PixelShaderHandle CreatePixelShader(const wchar_t* fileName, const char* entry);

  PixelShaderHandle CreatePixelShader(std::string& fileContents, const char* entry);

  GeometryShaderHandle CreateGeometryShader(const wchar_t* fileName, const char* entry);

  GeometryShaderHandle CreateGeometryShader(std::string& fileContents, const char* entry);

  ProgramHandle CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader);

  ProgramHandle CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader, GeometryShaderHandle gShader);

  void BindProgram(ProgramHandle handle);

  // Render Targets

  RenderTargetHandle CreateRenderTarget(float width, float height);

  void BindRenderTarget(RenderTargetHandle handle);

  void UnbindRenderTarget(RenderTargetHandle handle);

  void ClearRenderTarget(RenderTargetHandle handle, std::array<float, 4> color, bool clearDepth, bool clearStencil);

  TextureHandle GetTexture(RenderTargetHandle handle);

  // Samplers

  SamplerHandle CreateSampler(Filter filter = Filter::Linear, WrapMode wrapMode = WrapMode::Wrap);

  void BindSampler(SamplerHandle handle, ShaderType shader, int slot);

  // Textures

  TextureHandle CreateTexture(int width, int height, TextureFormat format, void* data);

  void BindTexture(TextureHandle, ShaderType shader, int slot);

  // Shader Constants

  ConstBufferHandle CreateConstantBuffer(uint32_t bufferSize);

  void BindConstantBuffer(ConstBufferHandle handle, const void* bufferData, ShaderType shader, int slot);
}

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