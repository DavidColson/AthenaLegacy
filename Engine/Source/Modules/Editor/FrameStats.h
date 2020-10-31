#pragma once

#include "Editor.h"

struct FrameStats : public EditorTool
{
	int frameStatsCounter = 0; // used so we only update framerate every few frames to make it less annoying to read
	double oldRealFrameTime;
	double oldObservedFrameTime;

	FrameStats();

	virtual void Update(Scene& scene, float deltaTime) override;
};