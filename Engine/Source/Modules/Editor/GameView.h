#pragma once

#include "Editor.h"
#include "Vec2.h"

struct GameView : public EditorTool
{
	GameView();

	virtual void Update(Scene& scene, float deltaTime) override;

    virtual void OnEditorResize(Vec2f newSize) override;
	
    virtual bool OnEvent(SDL_Event* event) override;
};