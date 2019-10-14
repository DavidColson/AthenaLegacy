#pragma once

struct Scene;

namespace Editor
{
	void SetCurrentScene(Scene* pScene);
	void ShowEditor(Scene* pScene, bool& shutdown, double realFrameTime, double observedFrameTime);
}