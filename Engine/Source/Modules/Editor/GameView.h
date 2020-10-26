#pragma once

#include "Editor.h"
#include "Vec2.h"

struct GameView : public EditorTool
{
	GameView();

	virtual void Update(Scene& scene) override;

    virtual void OnEditorResize(Vec2f newSize) override;
};