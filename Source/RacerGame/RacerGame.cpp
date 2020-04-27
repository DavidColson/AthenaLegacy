#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

#include <GltfLoader.h>
#include <Profiler.h>
#include <Matrix.h>
#include <SDL.h>
#include <Input/Input.h>

struct cbTransformBuf
{
	Matrixf wvp;
};

namespace {
	Mesh cubeMesh;
	Mesh monkey;
}

void CubeRenderSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	Matrixf view = Matrixf::Identity();
	Matrixf proj = Matrixf::Perspective(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight(), 0.1f, 100.0f, 60.0f);
	for (EntityID cams : SceneView<CCamera, CTransform>(scene))
	{
		CCamera* pCam = scene.Get<CCamera>(cams);
		CTransform* pTrans = scene.Get<CTransform>(cams);

		Matrixf translate = Matrixf::MakeTranslation(-pTrans->localPos);
		Quatf rotation = Quatf::MakeFromEuler(pTrans->localRot);
		view = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * translate;

		proj = Matrixf::Perspective(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight(), 0.1f, 100.0f, pCam->fov);
	}

	for (EntityID ent : SceneView<CRenderable, CTransform>(scene))
    {
		CRenderable* pCube = scene.Get<CRenderable>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

		Matrixf wvp = proj * view * pTrans->globalTransform;
		cbTransformBuf trans{ wvp };
		GfxDevice::BindConstantBuffer(pCube->constBuffer, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pCube->program);

		for (Primitive& prim : pCube->pMesh->primitives)
		{
			GfxDevice::SetTopologyType(prim.topologyType);
			GfxDevice::BindVertexBuffers(1, &prim.gfxVertBuffer);
			GfxDevice::BindIndexBuffer(prim.gfxIndexBuffer);
			GfxDevice::DrawIndexed((int)prim.indexBuffer.size(), 0, 0);
		}
	}
}

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
	scene.RegisterSystem(SystemPhase::Render, CubeRenderSystem);

	cubeMesh.name = "Cube";
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
	cubeMesh.primitives.push_back(cubePrimitive);
	cubeMesh.CreateGfxDeviceBuffers();

	// Material stuff!
	eastl::vector<VertexInputElement> layout;
	layout.push_back({"SV_POSITION",AttributeType::Float3});
	layout.push_back({"NORMAL",AttributeType::Float3});
	layout.push_back({"TEXCOORD_",AttributeType::Float2});
	layout.push_back({"COLOR_",AttributeType::Float4});
	VertexShaderHandle vShader = GfxDevice::CreateVertexShader(L"Shaders/VertColor.hlsl", "VSMain", layout, "CubeVertShader");
	PixelShaderHandle pShader = GfxDevice::CreatePixelShader(L"Shaders/VertColor.hlsl", "PSMain", "CubePixelShader");
	ProgramHandle program = GfxDevice::CreateProgram(vShader, pShader);
	ConstBufferHandle transformBuffer = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "CubeTransBuffer");



	EntityID camera = scene.NewEntity("Camera");
	scene.Assign<CCamera>(camera);
	scene.Assign<CTransform>(camera);

	// Make cube
	EntityID cube = scene.NewEntity("Cube");
	{
		CTransform* pTrans = scene.Assign<CTransform>(cube);
		pTrans->localPos = Vec3f(0.0f, 0.0f, -3.0f);
		pTrans->localSca = Vec3f(0.5f, 0.5f, 0.5f);
		
		CRenderable* pRenderable = scene.Assign<CRenderable>(cube);
		pRenderable->pMesh = &cubeMesh;
		pRenderable->program = program;
		pRenderable->constBuffer = transformBuffer;
	}

	// Make another cube
	EntityID cube2 = scene.NewEntity("Cube2");
	{
		CTransform* pTrans = scene.Assign<CTransform>(cube2);
		scene.SetParent(cube2, cube);
		pTrans->localPos = Vec3f(1.0f, 0.0f, -5.0f);
		pTrans->localSca = Vec3f(1.0f, 1.0f, 1.0f);
		
		CRenderable* pRenderable = scene.Assign<CRenderable>(cube2);
		pRenderable->pMesh = &cubeMesh;
		pRenderable->program = program;
		pRenderable->constBuffer = transformBuffer;
	}

	EntityID cube3 = scene.NewEntity("Cube3");
	{
		scene.SetParent(cube3, cube2);
		CTransform* pTrans = scene.Assign<CTransform>(cube3);
		pTrans->localPos = Vec3f(1.0f, 0.0f, -5.0f);
		pTrans->localSca = Vec3f(1.0f, 1.0f, 1.0f);

		CRenderable* pRenderable = scene.Assign<CRenderable>(cube3);
		pRenderable->pMesh = &cubeMesh;
		pRenderable->program = program;
		pRenderable->constBuffer = transformBuffer;
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
		GltfScene loadedScene = LoadGltf("Resources/Models/monkey.gltf");
		monkey = loadedScene.meshes[0];
		monkey.CreateGfxDeviceBuffers();


		EntityID triangle = scene.NewEntity("Monkey");
		CTransform* pTriangleTrans = scene.Assign<CTransform>(triangle);
		
		CRenderable* pRenderable = scene.Assign<CRenderable>(triangle);
		pRenderable->pMesh = &monkey;
		pRenderable->program = program;
		pRenderable->constBuffer = transformBuffer;
	}
}

int main(int argc, char *argv[])
{
	Engine::Initialize();

	Scene* pScene = new Scene();
	SetupScene(*pScene);

	// Run everything
	Engine::Run(pScene);

	return 0;
}