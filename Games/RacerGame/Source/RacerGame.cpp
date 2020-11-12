#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

#include <Mesh.h>
#include <Profiler.h>
#include <Matrix.h>
#include <SDL.h>
#include <Input/Input.h>
#include <Rendering/GameRenderer.h>
#include <AssetDatabase.h>
#include <SceneSerializer.h>
#include <Json.h>
#include <Path.h>

#include <Rendering/GfxDraw.h>

#include <FileSystem.h>

void CameraControlSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	GfxDraw::Line(Vec3f(0.0f, 0.0f, 2.0f), Vec3f(0.0f, 1.0f, 5.0f), 0.01f);
	GfxDraw::Line(Vec3f(2.0f, 0.0f, -1.0f), Vec3f(0.0f, 0.0f, 5.0f), 0.02f);
	GfxDraw::Line(Vec3f(2.0f, 0.0f, 4.0f), Vec3f(0.0f, 2.0f, 5.0f), 0.03f);

	for (EntityID cams : SceneIterator<CCamera, CTransform>(scene))
	{
		CCamera* pCam = scene.Get<CCamera>(cams);
		CTransform* pTrans = scene.Get<CTransform>(cams);

		const float camSpeed = 5.0f;

		Matrixf toCameraSpace = Quatf::MakeFromEuler(pTrans->localRot).ToMatrix();
		Vec3f right = toCameraSpace.GetRightVector().GetNormalized();
		if (Input::GetKeyHeld(SDL_SCANCODE_A))
			pTrans->localPos -= right * camSpeed * deltaTime;
		if (Input::GetKeyHeld(SDL_SCANCODE_D))
			pTrans->localPos += right * camSpeed * deltaTime;
			
		Vec3f forward = toCameraSpace.GetForwardVector().GetNormalized();
		if (Input::GetKeyHeld(SDL_SCANCODE_W))
			pTrans->localPos -= forward * camSpeed * deltaTime;
		if (Input::GetKeyHeld(SDL_SCANCODE_S))
			pTrans->localPos += forward * camSpeed * deltaTime;

		Vec3f up = toCameraSpace.GetUpVector().GetNormalized();
		if (Input::GetKeyHeld(SDL_SCANCODE_SPACE))
			pTrans->localPos += up * camSpeed * deltaTime;
		if (Input::GetKeyHeld(SDL_SCANCODE_LCTRL))
			pTrans->localPos -= up * camSpeed * deltaTime;

		if (Input::GetMouseInRelativeMode())
		{
			pCam->horizontalAngle -= 0.1f * deltaTime * Input::GetMouseDelta().x;
			pCam->verticalAngle -= 0.1f * deltaTime * Input::GetMouseDelta().y;
		}
		pTrans->localRot = Vec3f(pCam->verticalAngle, pCam->horizontalAngle, 0.0f);
	}
}

int main(int argc, char *argv[])
{
	EngineConfig config;
	config.windowName = "Racer Game";
	config.gameResourcesPath = "Games/RacerGame/Resources/";
	config.multiSamples = 1;

	Engine::Initialize(config);

	using namespace FileSys;

	// Custom mesh asset
	Mesh* pCubeMesh = new Mesh();
	pCubeMesh->name = "Cube";
	pCubeMesh->primitives.push_back(Primitive::NewCube());
	AssetDB::RegisterAsset(pCubeMesh, "cube");

	
	// Open the level we want to play
	JsonValue jsonScene = ParseJsonFile(FileSys::ReadWholeFile("Games/RacerGame/Resources/Levels/RacerGame.lvl"));
	Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);

	// Register systems
	Engine::SetSceneCreateCallback([](Scene& newScene) {
		newScene.RegisterSystem(SystemPhase::Update, CameraControlSystem);
	});

	// Run everything
	Engine::Run(pScene);

	return 0;
}