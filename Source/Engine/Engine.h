#pragma once

#include "SDL.h"

struct Scene;

namespace Engine
{
	void Initialize();
	void Run(Scene* pInitialScene);
	void SetActiveScene(Scene* pScene);
	void GetFrameRates(double& outReal, double& outLimited);
	void StartShutdown();
}