#pragma once

struct Scene;

namespace Editor
{
	void SetCurrentScene(Scene* pScene);
	void ShowEditor(bool& shutdown, double realFrameTime, double observedFrameTime);
}