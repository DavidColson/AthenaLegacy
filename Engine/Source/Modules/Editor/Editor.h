#pragma once

#include "Vec2.h"
#include "UUID.h"

struct Scene;
struct TextureHandle;
struct UpdateContext;
union SDL_Event;

struct EditorTool
{
	bool open { true };
	eastl::string menuName{ "unnamed tool" };
	virtual void Update(Scene& scene, UpdateContext& ctx) = 0;
	virtual void OnEditorResize(Vec2f newSize) {}
	virtual bool OnEvent(SDL_Event* event) { return false; }

	virtual ~EditorTool() {}
};

namespace Editor
{
	void Initialize(bool enabled);
	void PreUpdate();
	bool ProcessEvent(SDL_Event* event);
	TextureHandle DrawFrame(Scene& scene, UpdateContext& ctx);
	void Destroy();
	
	void ResizeEditorFrame(float width, float height);
	
	bool IsActive();
	void ToggleEditor();

	Uuid GetSelectedEntity();
	void SetSelectedEntity(Uuid entity);
}