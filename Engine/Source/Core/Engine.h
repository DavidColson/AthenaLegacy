#pragma once

#include "Vec2.h"

struct Scene;

enum class ResolutionStretchMode
{
	NoStretch,
	IgnoreAspect,
	KeepAspect,
	KeepWidth,
	KeepHeight,
	Expand
};
REFLECT_ENUM(ResolutionStretchMode)

struct EngineConfig
{
	REFLECT()

	eastl::string windowName{ "Athena" };
	
	// Rendering
	Vec2f windowResolution{ Vec2f( 1800.f, 1000.f ) };
	Vec2f baseGameResolution{ Vec2f( 1800.f, 1000.f ) };
	bool gameFramePointFilter{ true };
	ResolutionStretchMode resolutionStretchMode{ ResolutionStretchMode::NoStretch };
	int multiSamples{ 4 };
	
	// Editor
	bool bootInEditor{ true };

	// Assets
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

	bool IsInEditor();
}