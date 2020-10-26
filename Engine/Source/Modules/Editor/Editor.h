#pragma once

#include "Vec2.h"

struct Scene;
struct TextureHandle;
union SDL_Event;

struct EditorTool
{
	bool open { true };
	eastl::string menuName{ "unnamed tool" };
	virtual void Update(Scene& scene) = 0;
	virtual void OnEditorResize(Vec2f newSize) {}
};

namespace Editor
{
	void Initialize(bool enabled);
	void PreUpdate();
	void ProcessEvent(Scene& scene, SDL_Event* event);
	TextureHandle DrawFrame(Scene& scene, float deltaTime);
	void Destroy();
	
	void ResizeEditorFrame(float width, float height);
	
	bool IsActive();
	void ToggleEditor();

	EntityID GetSelectedEntity();
	void SetSelectedEntity(EntityID entity);
}