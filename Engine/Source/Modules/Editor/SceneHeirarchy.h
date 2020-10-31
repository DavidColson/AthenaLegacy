#pragma once

#include "Editor.h"

struct SceneHeirarchy : public EditorTool
{
	SceneHeirarchy();

	virtual void Update(Scene& scene, float deltaTime) override;
};