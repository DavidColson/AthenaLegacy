#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

#include <Mesh.h>
#include <Profiler.h>
#include <Matrix.h>
#include <SDL.h>
#include <Input/Input.h>
#include <Rendering/SceneDrawSystem.h>
#include <AssetDatabase.h>
#include <SceneSerializer.h>
#include <Json.h>
#include <Path.h>

#include <FileSystem.h>

void CameraControlSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	for (EntityID cams : SceneView<CCamera, CTransform>(scene))
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

	Engine::Initialize(config);

	using namespace FileSys;

	// Custom mesh asset
	Mesh* pCubeMesh = new Mesh();
	pCubeMesh->name = "Cube";
	Primitive cubePrimitive;
	cubePrimitive.vertBuffer = {
		Vert_PosNormTexCol{ Vec3f(-1.0f, -1.0f,  1.0f), Vec3f(), Vec2f(), Vec4f(1.0f, 0.0f, 0.0f, 1.0f) }, // Front bottom left
        Vert_PosNormTexCol{ Vec3f( 1.0f, -1.0f,  1.0f), Vec3f(), Vec2f(), Vec4f(0.0f, 1.0f, 0.0f, 1.0f) }, // Front bottom right
        Vert_PosNormTexCol{ Vec3f(-1.0f,  1.0f,  1.0f), Vec3f(), Vec2f(), Vec4f(1.0f, 0.0f, 1.0f, 1.0f) }, // Front top left
        Vert_PosNormTexCol{ Vec3f( 1.0f,  1.0f,  1.0f), Vec3f(), Vec2f(), Vec4f(1.0f, 1.0f, 1.0f, 1.0f) }, // Front top right

        Vert_PosNormTexCol{ Vec3f(-1.0f, -1.0f, -1.0f), Vec3f(), Vec2f(), Vec4f(1.0f, 0.0f, 0.0f, 1.0f) }, // Back bottom left
        Vert_PosNormTexCol{ Vec3f( 1.0f, -1.0f, -1.0f), Vec3f(), Vec2f(), Vec4f(0.0f, 1.0f, 0.0f, 1.0f) }, // Back bottom right
        Vert_PosNormTexCol{ Vec3f(-1.0f,  1.0f, -1.0f), Vec3f(), Vec2f(), Vec4f(1.0f, 0.0f, 1.0f, 1.0f) }, // Back top left
        Vert_PosNormTexCol{ Vec3f( 1.0f,  1.0f, -1.0f), Vec3f(), Vec2f(), Vec4f(1.0f, 1.0f, 1.0f, 1.0f) }  // Back top right
    };
	cubePrimitive.indexBuffer = {
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	};
	cubePrimitive.topologyType = TopologyType::TriangleStrip;
	pCubeMesh->primitives.push_back(cubePrimitive);
	pCubeMesh->CreateGfxDeviceBuffers();
	AssetDB::RegisterAsset(pCubeMesh, "cube");

	
	// Open the level we want to play
	JsonValue jsonScene = ParseJsonFile(FileSys::ReadWholeFile("Resources/Levels/RacerGame.lvl"));
	Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);

	// Register systems
	Engine::SetSceneCreateCallback([](Scene& newScene) {
		newScene.RegisterSystem(SystemPhase::Update, CameraControlSystem);
	});

	// Run everything
	Engine::Run(pScene);

	return 0;
}