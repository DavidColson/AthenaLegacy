#pragma once

struct IGame
{
	virtual void OnStart(Scene* pScene) = 0;
	virtual void OnFrame(Scene* pScene, float deltaTime) = 0;
	virtual void OnEnd(Scene* pScene) = 0;
};

