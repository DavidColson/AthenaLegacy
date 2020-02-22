#pragma once

#include "SDL.h"

struct Scene;

namespace Engine
{
	void Initialize();
	void Run(Scene* pInitialScene);
	void StartShutdown();

	void NewSceneCreated(Scene& scene);
	void SetActiveScene(Scene* pScene);
	void GetFrameRates(double& outReal, double& outLimited);
}