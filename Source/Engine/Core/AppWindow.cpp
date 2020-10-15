#include "AppWindow.h"

#include <SDL.h>
#include "Mesh.h"
#include "Log.h"

namespace
{
    SDL_Window* pWindow;

    Vec2f appSize;
    
    VertexBufferHandle vertBuffer;
    SamplerHandle textureSampler;
    VertexShaderHandle vertShader;
    PixelShaderHandle pixelShader;
    ProgramHandle program;
}

// ***********************************************************************

float AppWindow::GetWidth()
{
    return appSize.x;
}

// ***********************************************************************

float AppWindow::GetHeight()
{
    return appSize.y;
}

// ***********************************************************************

SDL_Window* AppWindow::GetSDLWindow()
{
    return pWindow;
}

// ***********************************************************************

void AppWindow::Create(float initialWidth, float initialHeight)
{
	Log::Info("App window size W: %.1f H: %.1f", initialWidth, initialHeight);

    pWindow = SDL_CreateWindow(
		"Athena",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(initialWidth),
		int(initialHeight),
		SDL_WINDOW_RESIZABLE
	);

    appSize = Vec2f(initialWidth, initialHeight);

	GfxDevice::Initialize(pWindow, initialWidth, initialHeight);

    // Setup rendering for full screen quad
    eastl::vector<Vert_PosTex> quadVertices = {
        Vert_PosTex{Vec3f(-1.0f, -1.0f, 0.0f), Vec2f(0.0f, 1.0f)},
        Vert_PosTex{Vec3f(1.f, -1.f, 0.0f), Vec2f(1.0f, 1.0f)},
        Vert_PosTex{Vec3f(-1.f, 1.f, 0.0f), Vec2f(0.0f, 0.0f)},
        Vert_PosTex{Vec3f(1.f, 1.f, 0.0f), Vec2f(1.0f, 0.0f)}
    };
    vertBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vert_PosTex), quadVertices.data(), "AppWindow quad");

    textureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Wrap, "AppWindow sampler");

    eastl::string drawTexturedQuadShader = "\
	struct VS_OUTPUT\
	{\
		float4 Pos : SV_POSITION;\
		float2 Tex : TEXCOORD0;\
	};\
	VS_OUTPUT VSMain(float4 inPos : POSITION, float2 inTex : TEXCOORD0)\
	{\
		VS_OUTPUT output;\
		output.Pos = inPos;\
        output.Tex = inTex;\
		return output;\
	}\
	Texture2D shaderTexture;\
	SamplerState textureSampler;\
	float4 PSMain(VS_OUTPUT input) : SV_TARGET\
	{\
		return shaderTexture.Sample(textureSampler, input.Tex);\
	}";

    eastl::vector<VertexInputElement> layout;
	layout.push_back({"POSITION", AttributeType::Float3});
	layout.push_back({"TEXCOORD", AttributeType::Float2});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(drawTexturedQuadShader, "VSMain", layout, "AppWindow");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(drawTexturedQuadShader, "PSMain", "AppWindow");
	program = GfxDevice::CreateProgram(vertShader, pixShader);
}

// ***********************************************************************

void AppWindow::RenderToWindow(TextureHandle frame)
{
    GfxDevice::SetBackBufferActive();
    GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });	
    GfxDevice::SetViewport(0.0f, 0.0f, appSize.x, appSize.y);

    GfxDevice::SetTopologyType(TopologyType::TriangleStrip);
    GfxDevice::BindProgram(program);
    GfxDevice::BindVertexBuffers(1, &vertBuffer);

    GfxDevice::BindTexture(frame, ShaderType::Pixel, 0);
    GfxDevice::BindSampler(textureSampler, ShaderType::Pixel, 0);

    GfxDevice::Draw(4, 0);

    GfxDevice::PresentBackBuffer();
    GfxDevice::ClearRenderState();
    GfxDevice::PrintQueuedDebugMessages();
}

// ***********************************************************************

void AppWindow::Resize(float width, float height)
{
    appSize = Vec2f(width, height);
    GfxDevice::ResizeBackBuffer(width, height);
}

// ***********************************************************************

void AppWindow::Destroy()
{
    GfxDevice::FreeVertexBuffer(vertBuffer);
	GfxDevice::FreeSampler(textureSampler);
	GfxDevice::FreeVertexShader(vertShader);
	GfxDevice::FreePixelShader(pixelShader);
	GfxDevice::FreeProgram(program);

    SDL_DestroyWindow(pWindow);
}