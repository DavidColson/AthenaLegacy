#pragma once

struct Scene;

namespace Editor
{
	void OnFrame(Scene& scene, bool& shutdown, double realFrameTime, double observedFrameTime);
}