#pragma once

#include "Editor.h"

struct Console : public EditorTool
{
    bool                scrollToBottom{ true };

	Console();
    
	virtual void Update(Scene& scene) override;
};