#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

#include <World.h>
#include <Entity.h>
#include <Systems.h>

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

#include <Rendering/SceneDrawSystem.h>

#include "LinearAllocator.h"

#include <FileSystem.h>

void CameraControlSystem(Scene& scene, float deltaTime)
{
	PROFILE();

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

	World world;

	{
		Entity* pMonkeyEnt = world.NewEntity("Monkey");
		Renderable* pRenderable = pMonkeyEnt->AddNewComponent<Renderable>();
		pRenderable->meshHandle = AssetHandle("Models/monkey.gltf:mesh_0");
		pRenderable->shaderHandle = AssetHandle("Shaders/VertColor.hlsl");
		pRenderable->SetLocalRotation(Vec3f(0.0f, 1.94f, 0.0f));
	}

	{
		Entity* pEntity = world.NewEntity("Cube");
		Renderable* pCubeRenderable = pEntity->AddNewComponent<Renderable>();
		pCubeRenderable->meshHandle = AssetHandle("cube");
		pCubeRenderable->shaderHandle = AssetHandle("Shaders/VertColor.hlsl");
		pCubeRenderable->SetLocalPosition(Vec3f(0.0f, 0.0f, -3.0f));
		pCubeRenderable->SetLocalScale(Vec3f(0.5f, 0.5f, 0.5f));

		Entity* pEntity2 = world.NewEntity("Cube2");
		Renderable* pCube2Renderable = pEntity->AddNewComponent<Renderable>();
		pCube2Renderable->SetParent(pCubeRenderable);
		pCube2Renderable->meshHandle = AssetHandle("cube");
		pCube2Renderable->shaderHandle = AssetHandle("Shaders/VertColor.hlsl");
		pCube2Renderable->SetLocalPosition(Vec3f(1.0f, 0.0f, -5.0f));
	
		Entity* pEntity3 = world.NewEntity("Cube3");
		Renderable* pRenderable = pEntity->AddNewComponent<Renderable>();
		pRenderable->SetParent(pCube2Renderable);
		pRenderable->meshHandle = AssetHandle("cube");
		pRenderable->shaderHandle = AssetHandle("Shaders/VertColor.hlsl");
		pRenderable->SetLocalPosition(Vec3f(1.0f, 0.0f, -5.0f));
	}
	{
		Entity* pEntity = world.NewEntity("NewCube");
		Renderable* pRenderable = pEntity->AddNewComponent<Renderable>();
		pRenderable->meshHandle = AssetHandle("cube");
		pRenderable->shaderHandle = AssetHandle("Shaders/VertColor.hlsl");
		pRenderable->SetLocalPosition(Vec3f(-4.9f, 2.79f, -8.19f));
		pRenderable->SetLocalRotation(Vec3f(0.0f, 0.3f, 0.0f));
		pRenderable->SetLocalScale(Vec3f(1.0f, 1.6f, 1.0f));
	}
	SceneDrawSystem* pDrawSystem = static_cast<SceneDrawSystem*>(world.AddGlobalSystem<SceneDrawSystem>());

	world.ActivateWorld();

	GameRenderer::SetSceneDrawSystem(pDrawSystem);

	Scene* pScene = new Scene();

	EntityID cameraEnt = pScene->NewEntity("Camera");
	pScene->Assign<CTransform>(cameraEnt);
	pScene->Assign<CCamera>(cameraEnt)->projection = ProjectionMode::Perspective;

	// Register systems
	Engine::SetSceneCreateCallback([](Scene& newScene) {
		newScene.RegisterSystem(SystemPhase::Update, CameraControlSystem);
	});

	// Run everything
	Engine::Run(pScene);

	return 0;
}