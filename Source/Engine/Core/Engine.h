#pragma once

#include "SDL.h"

struct Scene;

#define ENGINE_SINGLETON 0

namespace Engine
{
	void Initialize();
	void Run(Scene* pInitialScene);
	void StartShutdown();

	void NewSceneCreated(Scene& scene);
	void SetActiveScene(Scene* pScene);
	void GetFrameRates(double& outReal, double& outLimited);
}