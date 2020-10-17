#pragma once

#include "Editor.h"
#include "Vec2.h"

struct GameView : public EditorTool
{
    Vec2f windowSizeCache{ Vec2f(100.0f, 100.0f)};

	GameView();

	virtual void Update(Scene& scene) override;

    virtual void OnEditorResize(Vec2f newSize) override;
};