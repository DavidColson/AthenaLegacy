#pragma once

#include <array>
#include <vector>

#include "Maths/Vec2.h"
#include "Maths/Vec3.h"

// *************************
// Graphics Driver Interface
// *************************

struct SDL_Window;

class RenderFont;
struct Context;

struct Vertex
{
	Vertex(Vec3f _pos) : pos(_pos) {}

	Vec3f pos{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
	Vec2f texCoords{ Vec2f(0.0f, 0.0f) };
};

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
	Float3,
	Float2,
	InstanceTransform,
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
	unsigned int slot{ 0 };
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

	void DrawInstanced(int numVerts, int numInstances, int startVertex, int startInstance);

	// Blend States

	void SetBlending(const BlendingInfo& info);

	// Vertex Buffers

	VertexBufferHandle CreateVertexBuffer(size_t numElements, size_t _elementSize, void* data, const std::string& debugName = "");

	VertexBufferHandle CreateDynamicVertexBuffer(size_t numElements, size_t _elementSize, const std::string& debugName = "");

	void UpdateDynamicVertexBuffer(VertexBufferHandle handle, void* data, size_t dataSize);

	void BindVertexBuffers(size_t nBuffers, VertexBufferHandle* handles);

	// Index Buffers

	IndexBufferHandle CreateIndexBuffer(size_t numElements, void* data, const std::string& debugName = "");

	IndexBufferHandle CreateDynamicIndexBuffer(size_t numElements, const std::string& debugName = "");

	void UpdateDynamicIndexBuffer(IndexBufferHandle handle, void* data, size_t dataSize);

	int GetIndexBufferSize(IndexBufferHandle handle);

	void BindIndexBuffer(IndexBufferHandle handle);

	// Shaders And Programs

	VertexShaderHandle CreateVertexShader(const wchar_t* fileName, const char* entry, const std::vector<VertexInputElement>& inputLayout, const std::string& debugName = "");

	VertexShaderHandle CreateVertexShader(std::string& fileContents, const char* entry, const std::vector<VertexInputElement>& inputLayout, const std::string& debugName = "");

	PixelShaderHandle CreatePixelShader(const wchar_t* fileName, const char* entry, const std::string& debugName = "");

	PixelShaderHandle CreatePixelShader(std::string& fileContents, const char* entry, const std::string& debugName = "");

	GeometryShaderHandle CreateGeometryShader(const wchar_t* fileName, const char* entry, const std::string& debugName = "");

	GeometryShaderHandle CreateGeometryShader(std::string& fileContents, const char* entry, const std::string& debugName = "");

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

	SamplerHandle CreateSampler(Filter filter = Filter::Linear, WrapMode wrapMode = WrapMode::Wrap, const std::string& debugName = "");

	void BindSampler(SamplerHandle handle, ShaderType shader, int slot);

	// Textures

	TextureHandle CreateTexture(int width, int height, TextureFormat format, void* data, const std::string& debugName = "");

	void BindTexture(TextureHandle, ShaderType shader, int slot);

	// Shader Constants

	ConstBufferHandle CreateConstantBuffer(uint32_t bufferSize, const std::string& debugName = "");

	void BindConstantBuffer(ConstBufferHandle handle, const void* bufferData, ShaderType shader, int slot);

	// Debugging

	struct AutoEvent
	{
		AutoEvent(std::string label);
		~AutoEvent();
	};

	void SetDebugMarker(std::string label);
}

#define GFX_SCOPED_EVENT(label) GfxDevice::AutoEvent e(label)