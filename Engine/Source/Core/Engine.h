#pragma once

#include "Vec2.h"

struct Scene;

struct EngineConfig
{
	REFLECT()

	eastl::string windowName{ "Athena" };
	Vec2f windowResolution{ Vec2f( 1800.f, 1000.f ) };
	bool bootInEditor{ true };
	bool hotReloadingAssetsEnabled{ true };
	eastl::string gameResourcesPath{ "" };
	eastl::string engineResourcesPath{ "Engine/Resources/" };
};

namespace Engine
{
	void Initialize(const EngineConfig& config);
	void Initialize(const char* configFileName);
	void Run(Scene* pInitialScene);
	void StartShutdown();

	void SetActiveScene(Scene* pScene);
	void SetSceneCreateCallback(void (*pCallBackFunc)(Scene&));
	
	void NewSceneCreated(Scene& scene);
	void GetFrameRates(double& outReal, double& outLimited);

	const EngineConfig& GetConfig();
	void SaveConfig(const eastl::string& fileName);
}