#pragma once

#include "Editor.h"
#include "Vec2.h"

struct SceneView : public EditorTool
{
	SceneView();

	virtual void Update(Scene& scene, float deltaTime) override;

    virtual void OnEditorResize(Vec2f newSize) override;
};