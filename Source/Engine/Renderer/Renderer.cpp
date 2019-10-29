
#include "Renderer.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <stdio.h>
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

void Renderer::OnGameStart(Scene& scene)
{
	Context* pCtx = GfxDevice::GetContext();

	// Should be eventually moved to a material type when that exists
	VertexInputLayout layout;
  layout.AddElement("POSITION", AttributeType::float3);
  layout.AddElement("COLOR", AttributeType::float3);

  VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/Shader.hlsl", "VSMain", layout);
  PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/Shader.hlsl", "PSMain");
  GeometryShaderHandle geomShader = GfxDevice::CreateGeometryShader(L"Shaders/Shader.hlsl", "GSMain");

  pCtx->baseShaderProgram = GfxDevice::CreateProgram(vertShader, pixShader, geomShader);

	pCtx->pFontRender = new RenderFont("Resources/Fonts/Hyperspace/Hyperspace Bold.otf", 50);

	// *****************
	// Post processing
	// *****************

	for (EntityID ent : SceneView<CPostProcessing>(scene))
	{
		CPostProcessing* pp = scene.Get<CPostProcessing>(ent);

		for (int i = 0; i < 2; ++i)
		{
			pp->blurredFrame[i] = GfxDevice::CreateRenderTarget(pCtx->windowWidth / 2.0f, pCtx->windowHeight / 2.0f);
		}

		// Create post process shader data buffer
		D3D11_BUFFER_DESC ppdBufferDesc;
		ZeroMemory(&ppdBufferDesc, sizeof(ppdBufferDesc));
		ppdBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		ppdBufferDesc.ByteWidth = sizeof(CPostProcessing::cbPostProcessShaderData);
		ppdBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ppdBufferDesc.CPUAccessFlags = 0;
		ppdBufferDesc.MiscFlags = 0;
		GfxDevice::GetContext()->pDevice->CreateBuffer(&ppdBufferDesc, nullptr, &pp->pPostProcessDataBuffer);

		// Create bloom shader data buffer
		D3D11_BUFFER_DESC bloomBufferDesc;
		ZeroMemory(&bloomBufferDesc, sizeof(bloomBufferDesc));
		bloomBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bloomBufferDesc.ByteWidth = sizeof(CPostProcessing::cbBloomShaderData);
		bloomBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bloomBufferDesc.CPUAccessFlags = 0;
		bloomBufferDesc.MiscFlags = 0;
		GfxDevice::GetContext()->pDevice->CreateBuffer(&bloomBufferDesc, nullptr, &pp->pBloomDataBuffer);

		// Compile and create post processing shaders
		VertexInputLayout layout;
	  layout.AddElement("POSITION", AttributeType::float3);
	  layout.AddElement("COLOR", AttributeType::float3);
	  layout.AddElement("TEXCOORD", AttributeType::float2);

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
	ImGui_ImplSDL2_NewFrame(GfxDevice::GetContext()->pWindow);
	ImGui::NewFrame();
}

void Renderer::OnFrame(Scene& scene, float deltaTime)
{
	PROFILE();

	Context* pCtx = GfxDevice::GetContext();
	// RenderFonts -- Picks up text component data
	// RenderPostProcessing -- Need a scene post process component, lives on singleton?
	// RenderImgui - Editor components? Maybe something for future

	// ****************
	// Render Drawables
	// ****************
	{
		// First we draw the scene into a render target
		GfxDevice::BindRenderTarget(pCtx->preProcessedFrame);
		GfxDevice::ClearRenderTarget(pCtx->preProcessedFrame, { 0.0f, 0.f, 0.f, 1.0f }, true, true);
		GfxDevice::SetViewport(0.0f, 0.0f, pCtx->windowWidth, pCtx->windowHeight);

		// Set Shaders to active
		GfxDevice::BindProgram(pCtx->baseShaderProgram);
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
	pCtx->pFontRender->DrawSceneText(scene);

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
		GfxDevice::SetViewport(0.f, 0.f, pCtx->windowWidth / 2.0f, pCtx->windowHeight / 2.0f);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		// Bind bloom shader data
		GfxDevice::BindProgram(pp->bloomShaderProgram);
		GfxDevice::BindVertexBuffer(pCtx->fullScreenQuad);
		GfxDevice::BindSampler(pCtx->fullScreenTextureSampler, ShaderType::Pixel, 0);

		pp->bloomShaderData.resolution = Vec2f(900, 500);
		
		// Iteratively calculate bloom
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		int blurIterations = 8;
		for (int i = 0; i < blurIterations; ++i)
		{
			// Bind data
			float radius = float(blurIterations - i - 1) * 0.04f;
			pp->bloomShaderData.direction = i % 2 == 0 ? Vec2f(radius, 0.0f) : Vec2f(0.0f, radius);
			pCtx->pDeviceContext->UpdateSubresource(pp->pBloomDataBuffer, 0, nullptr, &pp->bloomShaderData, 0, 0);
			pCtx->pDeviceContext->PSSetConstantBuffers(0, 1, &(pp->pBloomDataBuffer));

			if (i == 0)
			{
				// First iteration, bind the plain, preprocessed frame
				RenderTarget& renderTarget = pCtx->renderTargets[pCtx->preProcessedFrame.id];
				pCtx->pDeviceContext->PSSetShaderResources(0, 1, &renderTarget.texture.pShaderResourceView);
			}
			else
			{
				GfxDevice::UnbindRenderTarget(pp->blurredFrame[(i + 1) % 2]);
				RenderTarget& renderTarget = pCtx->renderTargets[pp->blurredFrame[(i + 1) % 2].id];
				pCtx->pDeviceContext->PSSetShaderResources(0, 1, &renderTarget.texture.pShaderResourceView);
				GfxDevice::BindRenderTarget(pp->blurredFrame[i % 2]);
				GfxDevice::ClearRenderTarget(pp->blurredFrame[i % 2], { 0.0f, 0.f, 0.f, 1.0f }, false, false);
			}

			pCtx->pDeviceContext->Draw(4, 0);
		}

		// Now we'll actually render onto the backbuffer and do our final post process stage
		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0, 0, pCtx->windowWidth, pCtx->windowHeight);

		GfxDevice::BindProgram(pp->postProcessShaderProgram);
		GfxDevice::BindVertexBuffer(pCtx->fullScreenQuad);

		RenderTarget& renderTarget1 = pCtx->renderTargets[pCtx->preProcessedFrame.id];
		pCtx->pDeviceContext->PSSetShaderResources(0, 1, &renderTarget1.texture.pShaderResourceView);
		RenderTarget& renderTarget2 = pCtx->renderTargets[pp->blurredFrame[1].id];
		pCtx->pDeviceContext->PSSetShaderResources(1, 1, &renderTarget2.texture.pShaderResourceView);
		GfxDevice::BindSampler(pCtx->fullScreenTextureSampler, ShaderType::Pixel, 0);
		
		pp->postProcessShaderData.resolution = Vec2f(pCtx->windowWidth, pCtx->windowHeight);
		pp->postProcessShaderData.time = float(SDL_GetTicks()) / 1000.0f;
		pCtx->pDeviceContext->UpdateSubresource(pp->pPostProcessDataBuffer, 0, nullptr, &pp->postProcessShaderData, 0, 0);
		pCtx->pDeviceContext->PSSetConstantBuffers(0, 1, &(pp->pPostProcessDataBuffer));

		// Draw post processed frame
		pCtx->pDeviceContext->Draw(4, 0);
	}

	// If there was no post processing to do, just draw the pre-processed frame on screen
	if (!wasPostProcessing)
	{
		GfxDevice::SetBackBufferActive();
		GfxDevice::ClearBackBuffer({ 0.0f, 0.f, 0.f, 1.0f });
		GfxDevice::SetViewport(0, 0, pCtx->windowWidth, pCtx->windowHeight);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		GfxDevice::BindProgram(pCtx->fullScreenTextureProgram);
		GfxDevice::BindVertexBuffer(pCtx->fullScreenQuad);

		RenderTarget& renderTarget = pCtx->renderTargets[pCtx->preProcessedFrame.id];
		pCtx->pDeviceContext->PSSetShaderResources(0, 1, &renderTarget.texture.pShaderResourceView);
		GfxDevice::BindSampler(pCtx->fullScreenTextureSampler, ShaderType::Pixel, 0);

		pCtx->pDeviceContext->Draw(4, 0);
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