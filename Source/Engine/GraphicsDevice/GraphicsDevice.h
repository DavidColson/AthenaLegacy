#pragma once

#include <array>
#include <vector>

// *************************
// Graphics Driver Interface
// *************************

struct SDL_Window;

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
	// Need to fill this out
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

enum class BlendOp
{
	Add,
	Subtract,
	RevSubtract,
	Min,
	Max
};

enum class Blend
{
	Zero,
	One,
	SrcColor,
	InverseSrcColor,
	SrcAlpha,
	InverseSrcAlpha,
	DestAlpha,
	InverseDestAlpha,
	DestColor,
	InverseDestColor,
	BlendFactor,
	InverseBlendFactor
};

struct VertexInputElement
{
	const char* name;
	AttributeType type;
};

struct BlendingInfo
{
	bool enabled{ true };
	BlendOp colorOp{ BlendOp::Add };
	BlendOp alphaOp{ BlendOp::Add };
	Blend source{ Blend::One };
	Blend destination{ Blend::Zero };
	Blend sourceAlpha{ Blend::One };
	Blend destinationAlpha{ Blend::Zero };
	std::array<float, 4> blendFactor;
};

namespace GfxDevice
{
	void Initialize(SDL_Window* pWindow, float width, float height);

	void PrintQueuedDebugMessages();

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

	// Blend States

	void SetBlending(const BlendingInfo& info);

	// Vertex Buffers

	VertexBufferHandle CreateVertexBuffer(size_t numElements, size_t _elementSize, void* data, const std::string& debugName = "");

	VertexBufferHandle CreateDynamicVertexBuffer(size_t numElements, size_t _elementSize, const std::string& debugName = "");

	void UpdateDynamicVertexBuffer(VertexBufferHandle handle, void* data, size_t dataSize);

	void BindVertexBuffer(VertexBufferHandle handle);

	// Index Buffers

	IndexBufferHandle CreateIndexBuffer(size_t numElements, void* data, const std::string& debugName = "");

	IndexBufferHandle CreateDynamicIndexBuffer(size_t numElements, const std::string& debugName = "");

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

	RenderTargetHandle CreateRenderTarget(float width, float height, const std::string& debugName = "");

	void BindRenderTarget(RenderTargetHandle handle);

	void UnbindRenderTarget(RenderTargetHandle handle);

	void ClearRenderTarget(RenderTargetHandle handle, std::array<float, 4> color, bool clearDepth, bool clearStencil);

	TextureHandle GetTexture(RenderTargetHandle handle);

	// Samplers

	SamplerHandle CreateSampler(Filter filter = Filter::Linear, WrapMode wrapMode = WrapMode::Wrap);

	void BindSampler(SamplerHandle handle, ShaderType shader, int slot);

	// Textures

	TextureHandle CreateTexture(int width, int height, TextureFormat format, void* data, const std::string& debugName = "");

	void BindTexture(TextureHandle, ShaderType shader, int slot);

	// Shader Constants

	ConstBufferHandle CreateConstantBuffer(uint32_t bufferSize);

	void BindConstantBuffer(ConstBufferHandle handle, const void* bufferData, ShaderType shader, int slot);
}