#pragma once

#include "Vec2.h"

struct Scene;
struct TextureHandle;
union SDL_Event;

struct EditorTool
{
	bool open { true };
	eastl::string menuName{ "unnamed tool" };
	virtual void Update(Scene& scene, float deltaTime) = 0;
	virtual void OnEditorResize(Vec2f newSize) {}
	virtual bool OnEvent(SDL_Event* event) { return false; }

	virtual ~EditorTool() {}
};

namespace Editor
{
	void Initialize(bool enabled);
	void PreUpdate();
	bool ProcessEvent(Scene& scene, SDL_Event* event);
	TextureHandle DrawFrame(Scene& scene, float deltaTime);
	void Destroy();
	
	void ResizeEditorFrame(float width, float height);
	
	bool IsActive();
	void ToggleEditor();

	EntityID GetSelectedEntity();
	void SetSelectedEntity(EntityID entity);
}