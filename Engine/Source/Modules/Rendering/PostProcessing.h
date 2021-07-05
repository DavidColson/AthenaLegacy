#pragma once

struct FrameContext;

namespace PostProcessing
{
	void Initialize();
	void Destroy();

	void OnFrame(FrameContext& ctx, float deltaTime);
	void OnWindowResize(float newWidth, float newHeight);
}
