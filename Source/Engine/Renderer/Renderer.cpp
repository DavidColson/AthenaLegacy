
#include "Renderer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <ThirdParty/Imgui/imgui.h>
#include <ThirdParty/Imgui/examples/imgui_impl_sdl.h>
#include <ThirdParty/Imgui/examples/imgui_impl_dx11.h>

#include "Profiler.h"
#include "Log.h"
#include "RenderFont.h"
#include "DebugDraw.h"
#include "Scene.h"

REFLECT_BEGIN(CDrawable)
REFLECT_MEMBER(lineThickness)
REFLECT_END()

REFLECT_BEGIN(CPostProcessing)
REFLECT_END()

namespace
{
	// We render the scene into this framebuffer to give systems an opportunity to do 
  // post processing before we render into the backbuffer
  RenderTargetHandle preProcessedFrame;
  VertexBufferHandle fullScreenQuad;
  SamplerHandle fullScreenTextureSampler;
  ProgramHandle fullScreenTextureProgram; // simple shader program that draws a texture onscreen

  // Will eventually be a "material" type, assigned to drawables
  ProgramHandle baseShaderProgram;

  // Need a separate font render system, which pre processes text
  // into meshes
  RenderFont* pFontRender;
}

void Renderer::OnGameStart(Scene& scene)
{
	// Should be eventually moved to a material type when that exists
	{
		std::vector<VertexInputElement> baselayout;
	  baselayout.push_back({"POSITION", AttributeType::float3});
	  baselayout.push_back({"COLOR", AttributeType::float3 });
	  //baselayout.push_back({"TEXCOORD", AttributeType::float2});

	  VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/Shader.hlsl", "VSMain", baselayout);
	  PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/Shader.hlsl", "PSMain");
	  GeometryShaderHandle geomShader = GfxDevice::CreateGeometryShader(L"Shaders/Shader.hlsl", "GSMain");

	  baseShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader, geomShader);
	}

  // Create a program for drawing full screen quads to
  {
		std::vector<VertexInputElement> layout;
		layout.push_back({"POSITION", AttributeType::float3});
		layout.push_back({"COLOR", AttributeType::float3});
		layout.push_back({"TEXCOORD", AttributeType::float2});

		VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/FullScreenTexture.hlsl", "VSMain", layout);
		PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/FullScreenTexture.hlsl", "PSMain");

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
		fullScreenQuad = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vertex), quadVertices.data());

		fullScreenTextureProgram = GfxDevice::CreateProgram(vertShader, pixShader);
		fullScreenTextureSampler = GfxDevice::CreateSampler();
	}

  preProcessedFrame = GfxDevice::CreateRenderTarget(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

	pFontRender = new RenderFont("Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 50);

  DebugDraw::Detail::Init();

	// *****************
	// Post processing
	// *****************

	for (EntityID ent : SceneView<CPostProcessing>(scene))
	{
		CPostProcessing* pp = scene.Get<CPostProcessing>(ent);

		for (int i = 0; i < 2; ++i)
		{
			pp->blurredFrame[i] = GfxDevice::CreateRenderTarget(GfxDevice::GetWindowWidth() / 2.0f, GfxDevice::GetWindowHeight() / 2.0f);
		}

		// Create constant data buffers
		pp->postProcessDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(CPostProcessing::PostProcessShaderData));
		pp->bloomDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(CPostProcessing::BloomShaderData));

		// Compile and create post processing shaders
		std::vector<VertexInputElement> layout;
	  layout.push_back({"POSITION", AttributeType::float3});
	  layout.push_back({"COLOR", AttributeType::float3});
	  layout.push_back({"TEXCOORD", AttributeType::float2});

	  VertexShaderHandle vertPostProcessShader = GfxDevice::CreateVertexShader(L"Shaders/PostProcessing.hlsl", "VSMain", layout);
	  PixelShaderHandle pixPostProcessShader = GfxDevice::CreatePixelShader(L"Shaders/PostProcessing.hlsl", "PSMain");

	  pp->postProcessShaderProgram = GfxDevice::CreateProgram(vertPostProcessShader, pixPostProcessShader);

    VertexShaderHandle vertBloomShader = GfxDevice::CreateVertexShader(L"Shaders/Bloom.hlsl", "VSMain", layout);
	  PixelShaderHandle pixBloomShader = GfxDevice::CreatePixelShader(L"Shaders/Bloom.hlsl", "PSMain");

	  pp->bloomShaderProgram = GfxDevice::CreateProgram(vertBloomShader, pixBloomShader);
	}
}

void Renderer::OnFrameStart()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetWindow());
	ImGui::NewFrame();
}

void Renderer::OnFrame(Scene& scene, float deltaTime)
{
	PROFILE();
	// RenderFonts -- Picks up text component data
	// RenderPostProcessing -- Need a scene post process component, lives on singleton?
	// RenderImgui - Editor components? Maybe something for future

	// ****************
	// Render Drawables
	// ****************
	{
		// First we draw the scene into a render target
		GfxDevice::BindRenderTarget(preProcessedFrame);
		GfxDevice::ClearRenderTarget(preProcessedFrame, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
		GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

		// Set Shaders to active
		GfxDevice::BindProgram(baseShaderProgram);
		GfxDevice::SetTopologyType(TopologyType::LineStripAdjacency);


		for (EntityID ent : SceneView<CDrawable, CTransform>(scene))
		{
			CDrawable* pDrawable = scene.Get<CDrawable>(ent);
			CTransform* pTransform = scene.Get<CTransform>(ent);

			pDrawable->renderProxy.SetTransform(pTransform->pos, pTransform->rot, pTransform->sca);
			pDrawable->renderProxy.lineThickness = pDrawable->lineThickness;
			pDrawable->renderProxy.Draw();
		}
	}

	// *********
	// Draw Text
	// *********

	// Ideally font is just another mesh with a material to draw
	// So it would go through the normal channels, specialness is in it's material and probably
	// an earlier system that prepares the quads to render
	pFontRender->DrawSceneText(scene);

	// **********
	// Draw Debug 
	// **********

	DebugDraw::Detail::DrawQueue();

	// ***************
	// Post Processing
	// ***************

	bool wasPostProcessing = false;
	for (EntityID ent : SceneView<CPostProcessing>(scene))
	{
		wasPostProcessing = true;
		CPostProcessing* pp = scene.Get<CPostProcessing>(ent);

		GfxDevice::BindRenderTarget(pp->blurredFrame[0]);
		GfxDevice::ClearRenderTarget(pp->blurredFrame[0], { 0.0f, 0.f, 0.f, 1.0f }, false, false);
		GfxDevice::SetViewport(0.f, 0.f, GfxDevice::GetWindowWidth() / 2.0f, GfxDevice::GetWindowHeight() / 2.0f);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		// Bind bloom shader data
		GfxDevice::BindProgram(pp->bloomShaderProgram);
		GfxDevice::BindVertexBuffer(fullScreenQuad);
		GfxDevice::BindSampler(fullScreenTextureSampler, ShaderType::Pixel, 0);

		CPostProcessing::BloomShaderData bloomData;
		bloomData.resolution = Vec2f(900, 500);
		
		// Iteratively calculate bloom
		int blurIterations = 8;
		for (int i = 0; i < blurIterations; ++i)
		{
			// Bind data
			float radius = float(blurIterations - i - 1) * 0.04f;
			bloomData.direction = i % 2 == 0 ? Vec2f(radius, 0.0f) : Vec2f(0.0f, radius);
			GfxDevice::BindConstantBuffer(pp->bloomDataBuffer, &bloomData, ShaderType::Pixel, 0);

			if (i == 0)
			{
				// First iteration, bind the plain, preprocessed frame
				TextureHandle tex = GfxDevice::GetTexture(preProcessedFrame);
				GfxDevice::BindTexture(tex, ShaderType::Pixel, 0);
			}
			else
			{
				GfxDevice::UnbindRenderTarget(pp->blurredFrame[(i + 1) % 2]);
				TextureHandle tex = GfxDevice::GetTexture(pp->blurredFrame[(i + 1) % 2]);
				GfxDevice::BindTexture(tex, ShaderType::Pixel, 0);
				GfxDevice::BindRenderTarget(pp->blurredFrame[i % 2]);
				GfxDevice::ClearRenderTarget(pp->blurredFrame[i % 2], { 0.0f, 0.f, 0.f, 1.0f }, false, false);
			}

			GfxDevice::Draw(4, 0);
		}

		// Now we'll actually render onto the backbuffer and do our final post process stage
		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0, 0, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());

		GfxDevice::BindProgram(pp->postProcessShaderProgram);
		GfxDevice::BindVertexBuffer(fullScreenQuad);

		TextureHandle ppFrameTex = GfxDevice::GetTexture(preProcessedFrame);
		GfxDevice::BindTexture(ppFrameTex, ShaderType::Pixel, 0);
		TextureHandle blurFrameTex = GfxDevice::GetTexture(pp->blurredFrame[1]);
		GfxDevice::BindTexture(blurFrameTex, ShaderType::Pixel, 1);

		GfxDevice::BindSampler(fullScreenTextureSampler, ShaderType::Pixel, 0);
		
		CPostProcessing::PostProcessShaderData ppData;
		ppData.resolution = Vec2f(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());
		ppData.time = float(SDL_GetTicks()) / 1000.0f;
		GfxDevice::BindConstantBuffer(pp->postProcessDataBuffer, &ppData, ShaderType::Pixel, 0);

		// Draw post processed frame
		GfxDevice::Draw(4, 0);
	}

	// If there was no post processing to do, just draw the pre-processed frame on screen
	if (!wasPostProcessing)
	{
		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0, 0, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		GfxDevice::BindProgram(fullScreenTextureProgram);
		GfxDevice::BindVertexBuffer(fullScreenQuad);

		TextureHandle tex = GfxDevice::GetTexture(preProcessedFrame);
		GfxDevice::BindTexture(tex, ShaderType::Pixel, 0);

		GfxDevice::BindSampler(fullScreenTextureSampler, ShaderType::Pixel, 0);

		GfxDevice::Draw(4, 0);
	}

	// *******************
	// Draw Imgui Overlays
	// *******************
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// ***********************
	// Present Frame to screen
	// ***********************

	// switch the back buffer and the front buffer
	GfxDevice::PresentBackBuffer();
	GfxDevice::ClearRenderState();
}

void Renderer::OnGameEnd()
{
	// #TODO: Release things
}