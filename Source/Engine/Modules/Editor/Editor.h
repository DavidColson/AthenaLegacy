#pragma once

struct Scene;
struct TextureHandle;
union SDL_Event;

namespace Editor
{
	void Initialize();
	void PreUpdate();
	void ProcessEvent(Scene& scene, SDL_Event* event);
	TextureHandle DrawFrame(Scene& scene, float deltaTime);
	void Destroy();
	
	bool IsInEditor();
	void ToggleEditor();

	void SetGameFrame(TextureHandle texture);
	void FreeGameFrame();
}