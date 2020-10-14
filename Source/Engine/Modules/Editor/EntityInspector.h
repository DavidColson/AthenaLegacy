#pragma once

#include "Editor.h"

struct EntityInspector : public EditorTool
{
	EntityInspector();

	virtual void Update(Scene& scene) override;
};