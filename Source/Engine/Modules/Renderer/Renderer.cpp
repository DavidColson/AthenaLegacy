
#include "Renderer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "Profiler.h"
#include "Log.h"
#include "RenderFont.h"
#include "DebugDraw.h"
#include "Scene.h"
#include "Vec4.h"
#include "ParticlesSystem.h"

REFLECT_BEGIN(CDrawable)
REFLECT_MEMBER(lineThickness)
REFLECT_END()

REFLECT_BEGIN(CPostProcessing)
REFLECT_END()

namespace
{
	// We render the scene into this framebuffer to give systems an opportunity to do 
  // post processing before we render into the backbuffer
  RenderTargetHandle preProcessedFrame_deprecated;
  VertexBufferHandle fullScreenQuad;
  SamplerHandle fullScreenTextureSampler;
  ProgramHandle fullScreenTextureProgram; // simple shader program that draws a texture onscreen

  // Will eventually be a "material" type, assigned to drawables
  ProgramHandle baseShaderProgram;

  // Need a separate font render system, which pre processes text
  // into meshes
  RenderFont* pFontRender;
}

void Renderer::OnGameStart_Deprecated(Scene& scene)
{
	// Create a program for drawing full screen quads to
	// This has the most reason to be initialized in the rendering system for the entire game
	{
		std::vector<VertexInputElement> layout;
		layout.push_back({"POSITION", AttributeType::Float3});
		layout.push_back({"COLOR", AttributeType::Float3});
		layout.push_back({"TEXCOORD", AttributeType::Float2});

		VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/FullScreenTexture.hlsl", "VSMain", layout, "Fullscreen quad");
		PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/FullScreenTexture.hlsl", "PSMain", "Fullscreen quad");

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
		fullScreenQuad = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vertex), quadVertices.data(), "Fullscreen quad");

		fullScreenTextureProgram = GfxDevice::CreateProgram(vertShader, pixShader);
		fullScreenTextureSampler = GfxDevice::CreateSampler(Filter::Linear, WrapMode::Wrap, "Fullscreen texture");
	}

	preProcessedFrame_deprecated = GfxDevice::CreateRenderTarget(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight(), "Pre processed frame");

	// This should be part of an actual font pre-rendering system, which will initialize and manage itself and give actual commands to the core rendering queue
	pFontRender = new RenderFont("Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 50);

	DebugDraw::Detail::Init();
}

void Renderer::OnDrawableAdded(Scene& scene, EntityID ent)
{
	CDrawable& drawable = *(scene.Get<CDrawable>(ent));

	std::vector<VertexInputElement> baselayout;
	baselayout.push_back({"POSITION", AttributeType::Float3});
	baselayout.push_back({"COLOR", AttributeType::Float3 });
	baselayout.push_back({"TEXCOORD", AttributeType::Float2});

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/Shader.hlsl", "VSMain", baselayout, "Base shape");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/Shader.hlsl", "PSMain", "Base shape");
	GeometryShaderHandle geomShader = GfxDevice::CreateGeometryShader(L"Shaders/Shader.hlsl", "GSMain", "Base shape");

	drawable.baseProgram = GfxDevice::CreateProgram(vertShader, pixShader, geomShader);
	drawable.transformBuffer = GfxDevice::CreateConstantBuffer(sizeof(CDrawable::TransformData), scene.GetEntityName(ent));
}

void Renderer::OnPostProcessingAdded(Scene& scene, EntityID ent)
{
	CPostProcessing& pp = *(scene.Get<CPostProcessing>(ent));
	for (int i = 0; i < 2; ++i)
	{
		pp.blurredFrame[i] = GfxDevice::CreateRenderTarget(GfxDevice::GetWindowWidth() / 2.0f, GfxDevice::GetWindowHeight() / 2.0f, StringFormat("Blurred frame %i", i));
	}

	// Create constant data buffers
	pp.postProcessDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(CPostProcessing::PostProcessShaderData), "Post process shader data");
	pp.bloomDataBuffer = GfxDevice::CreateConstantBuffer(sizeof(CPostProcessing::BloomShaderData), "Bloom shader data");

	// Compile and create post processing shaders
	std::vector<VertexInputElement> layout;
	layout.push_back({"POSITION", AttributeType::Float3});
	layout.push_back({"COLOR", AttributeType::Float3});
	layout.push_back({"TEXCOORD", AttributeType::Float2});

	VertexShaderHandle vertPostProcessShader = GfxDevice::CreateVertexShader(L"Shaders/PostProcessing.hlsl", "VSMain", layout, "Post processing");
	PixelShaderHandle pixPostProcessShader = GfxDevice::CreatePixelShader(L"Shaders/PostProcessing.hlsl", "PSMain", "Post processing");

	pp.postProcessShaderProgram = GfxDevice::CreateProgram(vertPostProcessShader, pixPostProcessShader);

	VertexShaderHandle vertBloomShader = GfxDevice::CreateVertexShader(L"Shaders/Bloom.hlsl", "VSMain", layout, "Bloom");
	PixelShaderHandle pixBloomShader = GfxDevice::CreatePixelShader(L"Shaders/Bloom.hlsl", "PSMain", "Bloom");

	pp.bloomShaderProgram = GfxDevice::CreateProgram(vertBloomShader, pixBloomShader);
}

void Renderer::OnFrameStart(Scene& scene, float deltaTime)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetWindow());
	ImGui::NewFrame();
}

void Renderer::OnFrame(Scene& scene, float deltaTime)
{
	PROFILE();

	GfxDevice::SetBackBufferActive();
	GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
	GfxDevice::SetViewport(0.0f, 0.0f, GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight());
	
	// ****************
	// Render Drawables
	// ****************
	{
		GFX_SCOPED_EVENT("Rendering drawables");

		GfxDevice::SetTopologyType(TopologyType::LineStripAdjacency);

		for (EntityID ent : SceneView<CDrawable, CTransform>(scene))
		{
			if (scene.Has<CVisibility>(ent))
			{
				if (scene.Get<CVisibility>(ent)->visible == false)
					continue;
			}

			CDrawable* pDrawable = scene.Get<CDrawable>(ent);
			CTransform* pTransform = scene.Get<CTransform>(ent);

			// TODO: At some point there should be a shape/mesh data type which will create it's own buffers and they can be queried
			if (!IsValid(pDrawable->vertBuffer))
				pDrawable->vertBuffer = GfxDevice::CreateVertexBuffer(pDrawable->vertices.size(), sizeof(Vertex), pDrawable->vertices.data(), scene.GetEntityName(ent));

			if (!IsValid(pDrawable->indexBuffer))
				pDrawable->indexBuffer = GfxDevice::CreateIndexBuffer(pDrawable->indices.size(), pDrawable->indices.data(), scene.GetEntityName(ent));


			// Set vertex buffer as active
			GfxDevice::BindProgram(pDrawable->baseProgram);
			GfxDevice::BindVertexBuffers(1, &pDrawable->vertBuffer);
			GfxDevice::BindIndexBuffer(pDrawable->indexBuffer);

			Matrixf posMat = Matrixf::Translate(pTransform->pos);
			Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, pTransform->rot));
			Matrixf scaMat = Matrixf::Scale(pTransform->sca);
			Matrixf pivotAdjust = Matrixf::Translate(Vec3f(-0.5f, -0.5f, 0.0f));

			Matrixf world = posMat * rotMat * scaMat * pivotAdjust; // transform into world space
			Matrixf view = Matrixf::Translate(Vec3f(0.0f, 0.0f, 0.0f)); // transform into camera space

			Matrixf projection = Matrixf::Orthographic(0.f, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), -1.0f, 10.0f); // transform into screen space
			
			Matrixf wvp = projection * view * world;

			// TODO: Consider using a flag here for picking a shader, so we don't have to do memcpy twice
			CDrawable::TransformData trans{ wvp, pDrawable->lineThickness };
			GfxDevice::BindConstantBuffer(pDrawable->transformBuffer, &trans, ShaderType::Vertex, 0);
			GfxDevice::BindConstantBuffer(pDrawable->transformBuffer, &trans, ShaderType::Geometry, 0);

			GfxDevice::DrawIndexed(GfxDevice::GetIndexBufferSize(pDrawable->indexBuffer), 0, 0);
		}
	}

	// ****************
	// Render particles
	// ****************

	ParticlesSystem::OnFrame(scene, deltaTime);

	// *********
	// Render Text
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
	
	// Post processing system should be separated out. It should start by copying the current back buffer contents into a new texture.
	// That copied texture will be pre-processed frame. At it's end it should render everything back into the back buffer.
	// This means that the renderer file no longer needs to hold this full screen frame rendering setup anymore.
	// This decouples post processing from the flow of everything else. Since prior systems don't need to render into this pre-processed frame render target, 
	// Following that post processing should be moved to it's own file and system

	// Debug drawing should store it's queue in an engine singleton component and become a fully fledged system
	// Font rendering should should become it's own component and system sets, can render straight into the back buffer
	// Drawable rendering will go away and be replaced by Shape components that define vector shapes and will get drawn by a vector drawing system
	// (convenience functions can be made to do CreateRectangleShape(x, x, x...), which assign components and set them up)

	// After all this is done, the entire Renderer class will collapse and simply become a list of OnRender systems to execute
	bool wasPostProcessing = false;
	{
		GFX_SCOPED_EVENT("Doing post processing");	
		TextureHandle preProcessedFrame = GfxDevice::CopyAndResolveBackBuffer();

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
			GfxDevice::BindVertexBuffers(1, &fullScreenQuad);
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
					TextureHandle tex = preProcessedFrame;
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
			GfxDevice::BindVertexBuffers(1, &fullScreenQuad);

			TextureHandle ppFrameTex = preProcessedFrame;
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
	}

	// *******************
	// Draw Imgui Overlays
	// *******************

	{
		GFX_SCOPED_EVENT("Drawing imgui");
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	// ***********************
	// Present Frame to screen
	// ***********************

	// switch the back buffer and the front buffer
	GfxDevice::PresentBackBuffer();
	GfxDevice::ClearRenderState();
}