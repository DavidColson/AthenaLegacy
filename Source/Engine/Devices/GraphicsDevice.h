#pragma once

#include <array>
#include <vector>

#include <EASTL/bitset.h>
#include <EASTL/fixed_vector.h>

#include "Vec2.h"
#include "Vec3.h"

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

template<typename HandleType, int MaxHandles>
struct HandleAllocator
{
	HandleType NewHandle()
	{
		if (!freeHandles.empty())
		{
			uint16_t newId = freeHandles.back();
			freeHandles.pop_back();
			handleStates.set(newId, true);
			return HandleType{ newId };
		}
		uint16_t newId = highestUnallocatedHandle;
		highestUnallocatedHandle += 1;
		handleStates.set(newId, true);
		return HandleType{ newId };
	}

	void FreeHandle(HandleType handle)
	{
		if (handle.id == UINT16_MAX) return;
		handleStates.set(handle.id, false);
		freeHandles.push_back(handle.id);
	}

	bool IsValid(HandleType handle)
	{
		if (handle.id == UINT16_MAX) return false;
		return handleStates.test(handle.id);
	}

	uint16_t highestUnallocatedHandle{ 0 };
	eastl::bitset<MaxHandles> handleStates; // Stores whether each handle is valid
	eastl::fixed_vector<uint16_t, MaxHandles> freeHandles; // Stores currently unused handles
};

#define DECLARE_GFX_HANDLE(name)                                                       \
	struct name { uint16_t id{ UINT16_MAX }; };                                        \
	namespace GfxDevice { bool IsValid(name handle); }										

#define DEFINE_GFX_HANDLE(name) \
	bool GfxDevice::IsValid(name handle) { return pCtx->alloc##name.IsValid(handle); }

#define DEFINE_RESOURCE_POOLS(HandleType, CoreType)\
	HandleAllocator<HandleType, MAX_HANDLES> alloc##HandleType;\
	CoreType pool##CoreType[MAX_HANDLES];

#define INVALID_HANDLE { UINT16_MAX }

DECLARE_GFX_HANDLE(RenderTargetHandle)
DECLARE_GFX_HANDLE(VertexBufferHandle)
DECLARE_GFX_HANDLE(IndexBufferHandle)
DECLARE_GFX_HANDLE(PixelShaderHandle)
DECLARE_GFX_HANDLE(VertexShaderHandle)
DECLARE_GFX_HANDLE(GeometryShaderHandle)
DECLARE_GFX_HANDLE(ProgramHandle)
DECLARE_GFX_HANDLE(SamplerHandle)
DECLARE_GFX_HANDLE(TextureHandle)
DECLARE_GFX_HANDLE(ConstBufferHandle)
DECLARE_GFX_HANDLE(BlendStateHandle)

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

	void Destroy();

	void PrintQueuedDebugMessages();

	SDL_Window* GetWindow();

	float GetWindowWidth();

	float GetWindowHeight();

	void SetViewport(float x, float y, float width, float height);

	void ClearBackBuffer(std::array<float, 4> color);

	void SetTopologyType(TopologyType type);

	void SetBackBufferActive();

	void PresentBackBuffer();

	TextureHandle CopyAndResolveBackBuffer();

	void ClearRenderState();

	void DrawIndexed(int indexCount, int startIndex, int startVertex);

	void Draw(int numVerts, int startVertex);

	void DrawInstanced(int numVerts, int numInstances, int startVertex, int startInstance);

	// Blend States

	BlendStateHandle CreateBlendState(const BlendingInfo& info);
	
	void SetBlending(BlendStateHandle handle);

	void FreeBlendState(BlendStateHandle handle);

	// Vertex Buffers

	VertexBufferHandle CreateVertexBuffer(size_t numElements, size_t _elementSize, void* data, const std::string& debugName = "");

	VertexBufferHandle CreateDynamicVertexBuffer(size_t numElements, size_t _elementSize, const std::string& debugName = "");

	void UpdateDynamicVertexBuffer(VertexBufferHandle handle, void* data, size_t dataSize);

	void BindVertexBuffers(size_t nBuffers, VertexBufferHandle* handles);

	void FreeVertexBuffer(VertexBufferHandle handle);

	// Index Buffers

	IndexBufferHandle CreateIndexBuffer(size_t numElements, void* data, const std::string& debugName = "");

	IndexBufferHandle CreateDynamicIndexBuffer(size_t numElements, const std::string& debugName = "");

	void UpdateDynamicIndexBuffer(IndexBufferHandle handle, void* data, size_t dataSize);

	int GetIndexBufferSize(IndexBufferHandle handle);

	void BindIndexBuffer(IndexBufferHandle handle);

	void FreeIndexBuffer(IndexBufferHandle handle);

	// Shaders And Programs

	VertexShaderHandle CreateVertexShader(const wchar_t* fileName, const char* entry, const std::vector<VertexInputElement>& inputLayout, const std::string& debugName = "");

	VertexShaderHandle CreateVertexShader(std::string& fileContents, const char* entry, const std::vector<VertexInputElement>& inputLayout, const std::string& debugName = "");

	void FreeVertexShader(VertexShaderHandle handle);

	PixelShaderHandle CreatePixelShader(const wchar_t* fileName, const char* entry, const std::string& debugName = "");

	PixelShaderHandle CreatePixelShader(std::string& fileContents, const char* entry, const std::string& debugName = "");

	void FreePixelShader(PixelShaderHandle handle);

	GeometryShaderHandle CreateGeometryShader(const wchar_t* fileName, const char* entry, const std::string& debugName = "");

	GeometryShaderHandle CreateGeometryShader(std::string& fileContents, const char* entry, const std::string& debugName = "");

	void FreeGeometryShader(GeometryShaderHandle handle);

	ProgramHandle CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader);

	ProgramHandle CreateProgram(VertexShaderHandle vShader, PixelShaderHandle pShader, GeometryShaderHandle gShader);

	void BindProgram(ProgramHandle handle);

	void FreeProgram(ProgramHandle handle, bool freeBoundShaders = true); 

	// Render Targets

	RenderTargetHandle CreateRenderTarget(float width, float height, const std::string& debugName = "");

	void BindRenderTarget(RenderTargetHandle handle);

	void UnbindRenderTarget(RenderTargetHandle handle);

	void ClearRenderTarget(RenderTargetHandle handle, std::array<float, 4> color, bool clearDepth, bool clearStencil);

	TextureHandle GetTexture(RenderTargetHandle handle);

	void FreeRenderTarget(RenderTargetHandle handle);

	// Samplers

	SamplerHandle CreateSampler(Filter filter = Filter::Linear, WrapMode wrapMode = WrapMode::Wrap, const std::string& debugName = "");

	void BindSampler(SamplerHandle handle, ShaderType shader, int slot);

	void FreeSampler(SamplerHandle handle);

	// Textures

	TextureHandle CreateTexture(int width, int height, TextureFormat format, void* data, const std::string& debugName = "");

	void BindTexture(TextureHandle handle, ShaderType shader, int slot);

	void FreeTexture(TextureHandle handle);

	// Shader Constants

	ConstBufferHandle CreateConstantBuffer(uint32_t bufferSize, const std::string& debugName = "");

	void BindConstantBuffer(ConstBufferHandle handle, const void* bufferData, ShaderType shader, int slot);

	void FreeConstBuffer(ConstBufferHandle handle);

	// Debugging

	struct AutoEvent
	{
		AutoEvent(std::string label);
		~AutoEvent();
	};

	void SetDebugMarker(std::string label);
}

#define GFX_SCOPED_EVENT(label) GfxDevice::AutoEvent e(label)