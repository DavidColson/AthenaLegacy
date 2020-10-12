#pragma once

struct Scene;
struct TextureHandle;

namespace Editor
{
	void OnFrame(Scene& scene, float deltaTime);
	bool IsInEditor();
	void SetGameFrame(TextureHandle texture);
	void FreeGameFrame();
}