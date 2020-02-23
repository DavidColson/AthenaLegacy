
#include "Renderer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <Imgui/imgui.h>
#include <Imgui/examples/imgui_impl_sdl.h>
#include <Imgui/examples/imgui_impl_dx11.h>

#include "Profiler.h"
#include "Log.h"
#include "FontSystem.h"
#include "DebugDraw.h"
#include "Scene.h"
#include "Vec4.h"
#include "ParticlesSystem.h"
#include "PostProcessingSystem.h"
#include "ShapesSystem.h"

namespace
{
  // Need a separate font render system, which pre processes text
  // into meshes
  RenderFont* pFontRender;
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
	// Render Shapes
	// ****************
	
	Shapes::OnFrame(scene, deltaTime);

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
	FontSystem::OnFrame(scene, deltaTime);

	// **********
	// Draw Debug 
	// **********

	DebugDraw::OnFrame(scene, deltaTime);

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
	PostProcessingSystem::OnFrame(scene, deltaTime);


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