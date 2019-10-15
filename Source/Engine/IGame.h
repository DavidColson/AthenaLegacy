#pragma once

struct Scene;

struct IGame
{
	virtual void OnStart(Scene& scene) = 0;
	virtual void OnFrame(Scene& scene, float deltaTime) = 0;
	virtual void OnEnd(Scene& scene) = 0;
};

