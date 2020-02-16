#pragma once

struct Scene;

struct IGame
{
	virtual void OnFrame(Scene& scene, float deltaTime) = 0;
};

