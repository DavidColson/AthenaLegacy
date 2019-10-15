#pragma once

struct Scene;

namespace Editor
{
	void ShowEditor(Scene& scene, bool& shutdown, double realFrameTime, double observedFrameTime);
}