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
		GfxDevice::SetTopologyType(pCube->type);

		GfxDevice::BindVertexBuffers(1, &pCube->vBuffer);
		GfxDevice::BindIndexBuffer(pCube->iBuffer);

		GfxDevice::DrawIndexed(pCube->indexCount, 0, 0);
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

	EntityID camera = scene.NewEntity("Camera");
	scene.Assign<CCamera>(camera);
	scene.Assign<CTransform>(camera);

	EntityID cube = scene.NewEntity("Cube");
	CRenderable* pCube = scene.Assign<CRenderable>(cube);
	CTransform* pTrans = scene.Assign<CTransform>(cube);
	pTrans->localPos = Vec3f(0.0f, 0.0f, -3.0f);
	pTrans->localSca = Vec3f(0.5f, 0.5f, 0.5f);
	
	eastl::vector<VertexInputElement> layout;
	layout.push_back({"SV_POSITION",AttributeType::Float3});
	layout.push_back({"COLOR", AttributeType::Float3});

	VertexShaderHandle vShader = GfxDevice::CreateVertexShader(L"Shaders/CubeShader.hlsl", "VSMain", layout, "CubeVertShader");
	PixelShaderHandle pShader = GfxDevice::CreatePixelShader(L"Shaders/CubeShader.hlsl", "PSMain", "CubePixelShader");

	pCube->program = GfxDevice::CreateProgram(vShader, pShader);



	eastl::vector<Vertex> cubeVerts = {
		Vertex(Vec3f(-1.0f, -1.0f,  1.0f)), // Front bottom left
        Vertex(Vec3f( 1.0f, -1.0f,  1.0f)), // Front bottom right
        Vertex(Vec3f(-1.0f,  1.0f,  1.0f)), // Front top left
        Vertex(Vec3f( 1.0f,  1.0f,  1.0f)), // Front top right

        Vertex(Vec3f(-1.0f, -1.0f, -1.0f)), // Back bottom left
        Vertex(Vec3f( 1.0f, -1.0f, -1.0f)), // Back bottom right
        Vertex(Vec3f(-1.0f,  1.0f, -1.0f)), // Back top left
        Vertex(Vec3f( 1.0f,  1.0f, -1.0f))  // Back top right
    };
	cubeVerts[4].col = Vec3f(1.0f, 0.0f, 0.0f);
	cubeVerts[5].col = Vec3f(0.0f, 1.0f, 0.0f);
	cubeVerts[6].col = Vec3f(1.0f, 0.0f, 1.0f);
	cubeVerts[7].col = Vec3f(1.0f, 1.0f, 1.0f);
	cubeVerts[0].col = Vec3f(1.0f, 0.0f, 0.0f);
	cubeVerts[1].col = Vec3f(0.0f, 1.0f, 0.0f);
	cubeVerts[2].col = Vec3f(1.0f, 0.0f, 1.0f);
	cubeVerts[3].col = Vec3f(1.0f, 1.0f, 1.0f);
	
	Mesh cubeMesh;
	cubeMesh.name = "Cube";
	Primitive cubePrimitive;

	pCube->vBuffer = GfxDevice::CreateVertexBuffer(8, sizeof(Vertex), cubeVerts.data(), "CubeVertBuffer");

	eastl::vector<int> cubeIndices = {
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	};
	pCube->iBuffer = GfxDevice::CreateIndexBuffer(14, IndexFormat::UInt, cubeIndices.data(), "CubeIndexBuffer");
	pCube->indexCount = (int)cubeIndices.size();

	pCube->constBuffer = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "CubeTransBuffer");

	EntityID cube2 = scene.NewEntity("Cube2");
	CRenderable* pCube2 = scene.Assign<CRenderable>(cube2);
	scene.SetParent(cube2, cube);
	CTransform* pTrans2 = scene.Assign<CTransform>(cube2);
	pTrans2->localPos = Vec3f(1.0f, 0.0f, -5.0f);
	pTrans2->localSca = Vec3f(1.0f, 1.0f, 1.0f);
	*pCube2 = *pCube;

	EntityID cube3 = scene.NewEntity("Cube3");
	CRenderable* pCube3 = scene.Assign<CRenderable>(cube3);
	scene.SetParent(cube3, cube2);
	CTransform* pTrans3 = scene.Assign<CTransform>(cube3);
	pTrans3->localPos = Vec3f(1.0f, 0.0f, -5.0f);
	pTrans3->localSca = Vec3f(1.0f, 1.0f, 1.0f);
	*pCube3 = *pCube;

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

	//scene.UnsetParent(cube2, cube);

	GltfScene loadedScene = LoadGltf("Resources/Models/monkey.gltf");

	EntityID triangle = scene.NewEntity("Monkey");
	CTransform* pTriangleTrans = scene.Assign<CTransform>(triangle);
	CRenderable* pRenderable = scene.Assign<CRenderable>(triangle);
	
	eastl::vector<VertexInputElement> trianglelayout;
	trianglelayout.push_back({"SV_POSITION",AttributeType::Float3});
	trianglelayout.push_back({"NORMAL",AttributeType::Float3});
	trianglelayout.push_back({"TEXCOORD_",AttributeType::Float2});
	trianglelayout.push_back({"COLOR_",AttributeType::Float4});

	VertexShaderHandle gvShader = GfxDevice::CreateVertexShader(L"Shaders/BasicGltfShader.hlsl", "VSMain", trianglelayout, "GltfVertShader");
	PixelShaderHandle gpShader = GfxDevice::CreatePixelShader(L"Shaders/BasicGltfShader.hlsl", "PSMain", "GltfPixelShader");
	pRenderable->program = GfxDevice::CreateProgram(gvShader, gpShader);
	pRenderable->constBuffer = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "GltfTransBuffer");

	loadedScene.meshes[0].CreateGfxDeviceBuffers();
	pRenderable->vBuffer = loadedScene.meshes[0].primitives[0].gfxVertBuffer;
	pRenderable->iBuffer = loadedScene.meshes[0].primitives[0].gfxIndexBuffer;
	pRenderable->indexCount = (int)loadedScene.meshes[0].primitives[0].indexBuffer.size();
	pRenderable->type = loadedScene.meshes[0].primitives[0].topologyType;
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