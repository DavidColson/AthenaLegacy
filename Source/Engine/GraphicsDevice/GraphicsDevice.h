#pragma once

#include <array>
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

  // This is more like a program than a shader
  // Probably rename it
  struct Shader
  {
    ID3D11VertexShader* pVertexShader{ nullptr };
    ID3D11GeometryShader* pGeometryShader{ nullptr };
    ID3D11PixelShader* pPixelShader{ nullptr };

    // This should be part of the vertex buffer
    ID3D11InputLayout* pVertLayout{ nullptr };

    // Should be part of the index buffer no?
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    void Bind();
  };

  Shader LoadShaderFromFile(const wchar_t* shaderName, bool hasGeometryShader);
  Shader LoadShaderFromText(std::string shaderContents, bool withTexCoords = true);

  // #TODO: Resources should have private constructors, and are created using GfxDevice Create* functions
  struct RenderTarget
  {
    GfxDevice::Texture2D texture;
    ID3D11RenderTargetView* pView{ nullptr };
    GfxDevice::Texture2D depthStencilTexture;
    ID3D11DepthStencilView* pDepthStencilView { nullptr };

    void Init(float width, float height);
    void SetActive();
    void UnsetActive();
    void ClearView(std::array<float, 4> color, bool clearDepth, bool clearStencil);
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
  ID3D11Buffer * pFullScreenVertBuffer{ nullptr };
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