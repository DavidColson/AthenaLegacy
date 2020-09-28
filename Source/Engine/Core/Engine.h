#pragma once

struct Scene;

namespace Engine
{
	void Initialize();
	void Run(Scene* pInitialScene);
	void StartShutdown();

	void SetActiveScene(Scene* pScene);
	void SetSceneCreateCallback(void (*pCallBackFunc)(Scene&));
	
	void NewSceneCreated(Scene& scene);
	void GetFrameRates(double& outReal, double& outLimited);
}