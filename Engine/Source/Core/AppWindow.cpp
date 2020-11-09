#include "AppWindow.h"

#include <SDL.h>

#include "Engine.h"
#include "Mesh.h"
#include "Log.h"

namespace
{
    SDL_Window* pWindow;

    Vec2f appSize;
    
    VertexBufferHandle verticesBuffer;
    VertexBufferHandle vertTexCoordsBuffer;

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

void BuildFrameVertexBuffer(float windowAspectRatio, float gameAspectRatio, bool forceFullFrame = false)
{
    float x = 1.0f, y = x;

    if (!forceFullFrame)
    {
        switch (Engine::GetConfig().resolutionStretchMode)
        {
        case ResolutionStretchMode::KeepAspect:
        {
            if (windowAspectRatio < gameAspectRatio)
                y = (1 / gameAspectRatio) * windowAspectRatio;
            else
                x = gameAspectRatio * (1 / windowAspectRatio);
            break;
        }
        case ResolutionStretchMode::KeepWidth:
        {
            if (windowAspectRatio > gameAspectRatio)
                x = gameAspectRatio * (1 / windowAspectRatio);
            break;
        }
        case ResolutionStretchMode::KeepHeight:
        {
            if (windowAspectRatio < gameAspectRatio)
                y = (1 / gameAspectRatio) * windowAspectRatio;
            break;
        }
        default:
            break;
        }
    }


    GfxDevice::FreeVertexBuffer(verticesBuffer);
    eastl::vector<Vec3f> quadVertices = {
        Vec3f(-x, -y, 0.0f),
        Vec3f(x, -y, 0.0f),
        Vec3f(-x, y, 0.0f),
        Vec3f(x, y, 0.0f)
    };
    verticesBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vec3f), quadVertices.data(), "AppWindow quad verts");

    GfxDevice::FreeVertexBuffer(vertTexCoordsBuffer);
    eastl::vector<Vec2f> quadTexCoords = {
        Vec2f(0.0f, 1.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(0.0f, 0.0f),
        Vec2f(1.0f, 0.0f)
    };
    vertTexCoordsBuffer = GfxDevice::CreateVertexBuffer(quadTexCoords.size(), sizeof(Vec2f), quadTexCoords.data(), "AppWindow quad texcoords");
}

// ***********************************************************************

void AppWindow::Create(float initialWidth, float initialHeight, const eastl::string& windowName)
{
	Log::Info("App window size W: %.1f H: %.1f", initialWidth, initialHeight);

    pWindow = SDL_CreateWindow(
		windowName.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		int(initialWidth),
		int(initialHeight),
		SDL_WINDOW_RESIZABLE
	);

    appSize = Vec2f(initialWidth, initialHeight);

	GfxDevice::Initialize(pWindow, initialWidth, initialHeight);

    // Setup rendering for full screen quad
    BuildFrameVertexBuffer(initialWidth / initialHeight, Engine::GetConfig().baseGameResolution.x / Engine::GetConfig().baseGameResolution.y, Engine::GetConfig().bootInEditor);

    if (Engine::GetConfig().gameFramePointFilter)
        textureSampler = GfxDevice::CreateSampler(Filter::Point, WrapMode::Wrap, "AppWindow sampler");
    else
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

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(drawTexturedQuadShader, "VSMain", "AppWindow");
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
    GfxDevice::BindVertexBuffers(0, 1, &verticesBuffer);
    GfxDevice::BindVertexBuffers(1, 1, &vertTexCoordsBuffer);

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
    BuildFrameVertexBuffer(width / height, Engine::GetConfig().baseGameResolution.x / Engine::GetConfig().baseGameResolution.y, Engine::IsInEditor());
}

// ***********************************************************************

void AppWindow::Destroy()
{
    GfxDevice::FreeVertexBuffer(verticesBuffer);
    GfxDevice::FreeVertexBuffer(vertTexCoordsBuffer);
	GfxDevice::FreeSampler(textureSampler);
	GfxDevice::FreeVertexShader(vertShader);
	GfxDevice::FreePixelShader(pixelShader);
	GfxDevice::FreeProgram(program);

    SDL_DestroyWindow(pWindow);
}