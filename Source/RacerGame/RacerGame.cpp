#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

#include <Mesh.h>
#include <Profiler.h>
#include <Matrix.h>
#include <SDL.h>
#include <Input/Input.h>
#include <Rendering/SceneDrawSystem.h>
#include <SceneSerializer.h>
#include <AssetDatabase.h>
#include <Json.h>

#include <FileSys.h>

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

void SetupScene(Scene& scene)
{
	scene.RegisterSystem(SystemPhase::Update, CameraControlSystem);

	EntityID camera = scene.NewEntity("Camera");
	scene.Assign<CCamera>(camera)->fov = 102;
	scene.Assign<CTransform>(camera);

	// Make cube
	EntityID cube = scene.NewEntity("Cube");
	{
		CTransform* pTrans = scene.Assign<CTransform>(cube);
		pTrans->localPos = Vec3f(0.0f, 0.0f, -3.0f);
		pTrans->localSca = Vec3f(0.5f, 0.5f, 0.5f);
		
		CRenderable* pRenderable = scene.Assign<CRenderable>(cube);
		pRenderable->shaderHandle = AssetHandle("Resources/Shaders/VertColor.hlsl");
		pRenderable->meshHandle = AssetHandle("cube");
	}

	// Make another cube
	EntityID cube2 = scene.NewEntity("Cube2");
	{
		CTransform* pTrans = scene.Assign<CTransform>(cube2);
		scene.SetParent(cube2, cube);
		pTrans->localPos = Vec3f(1.0f, 0.0f, -5.0f);
		pTrans->localSca = Vec3f(1.0f, 1.0f, 1.0f);
		
		CRenderable* pRenderable = scene.Assign<CRenderable>(cube2);
		pRenderable->shaderHandle = AssetHandle("Resources/Shaders/VertColor.hlsl");
		pRenderable->meshHandle = AssetHandle("cube");
	}

	EntityID cube3 = scene.NewEntity("Cube3");
	{
		scene.SetParent(cube3, cube2);
		CTransform* pTrans = scene.Assign<CTransform>(cube3);
		pTrans->localPos = Vec3f(1.0f, 0.0f, -5.0f);
		pTrans->localSca = Vec3f(1.0f, 1.0f, 1.0f);

		CRenderable* pRenderable = scene.Assign<CRenderable>(cube3);
		pRenderable->shaderHandle = AssetHandle("Resources/Shaders/VertColor.hlsl");
		pRenderable->meshHandle = AssetHandle("cube");
	}

	for(int i = 0; i < 4; i++)
	{
		EntityID newEnt = scene.NewEntity("ChildEnt");
		scene.SetParent(newEnt, cube);
	}

	for(int i = 0; i < 2; i++)
	{
		EntityID newEnt = scene.NewEntity("ChildEnt");
		scene.SetParent(newEnt, cube2);
	}

	{
		EntityID triangle = scene.NewEntity("Monkey");
		CTransform* pTriangleTrans = scene.Assign<CTransform>(triangle);
		
		CRenderable* pRenderable = scene.Assign<CRenderable>(triangle);
		pRenderable->shaderHandle = AssetHandle("Resources/Shaders/VertColor.hlsl");
		pRenderable->meshHandle = AssetHandle("Resources/Models/monkey.gltf:mesh_0");
	}

	FileSys::FileHandle fHandle = FileSys::open("SerializeRacerScene.txt");
	JsonValue jsonScene = SceneSerializer::ToJson(scene);
	fHandle.writeFile(SerializeJsonValue(jsonScene));
}

struct InnerStruct
{
	float thisIsAFloat{ 22.0f };
	int thisIsAnInt{ 6 };

	REFLECT();
};

struct Component
{
  int myInt{ 5 };
  int mySecondInt{ 3 };

  float myFloat{ 21.412312731f };
  double myDouble{ 21.4123127311928746123 };

  eastl::string myString{ "Ducks" };
  bool myBool = false;

  EntityID anEntityId;

  AssetHandle monkey{ "Resources/Models/monkey.gltf:mesh_0" };

  InnerStruct myInnerStruct;

  REFLECT()
};

REFLECT_BEGIN(InnerStruct)
REFLECT_MEMBER(thisIsAFloat)
REFLECT_MEMBER(thisIsAnInt)
REFLECT_END()

REFLECT_BEGIN(Component)
REFLECT_MEMBER(myInt)
REFLECT_MEMBER(mySecondInt)
REFLECT_MEMBER(myFloat)
REFLECT_MEMBER(myDouble)
REFLECT_MEMBER(myString)
REFLECT_MEMBER(myBool)
REFLECT_MEMBER(anEntityId)
REFLECT_MEMBER(monkey)
REFLECT_MEMBER(myInnerStruct)
REFLECT_END()

int main(int argc, char *argv[])
{
	Engine::Initialize();

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

	// Scene* pScene = new Scene();
	// SetupScene(*pScene);

	FileSys::FileHandle fHandle = FileSys::open("SerializeRacerScene.txt");
	JsonValue jsonScene = ParseJsonFile(fHandle.readFile());
	Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);
	pScene->RegisterSystem(SystemPhase::Update, CameraControlSystem);


	// Run everything
	Engine::Run(pScene);

	return 0;
}