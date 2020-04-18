#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

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

	for (EntityID ent : SceneView<CCube, CTransform>(scene))
    {
		CCube* pCube = scene.Get<CCube>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

		Matrixf wvp = proj * view * pTrans->globalTransform;
		cbTransformBuf trans{ wvp };
		GfxDevice::BindConstantBuffer(pCube->constBuffer, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pCube->program);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		GfxDevice::BindVertexBuffers(1, &pCube->vBuffer);
		GfxDevice::BindIndexBuffer(pCube->iBuffer);

		GfxDevice::DrawIndexed(14, 0, 0);
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
			pTrans->localPos += forward * camSpeed * deltaTime;
		if (Input::GetKeyHeld(SDL_SCANCODE_S))
			pTrans->localPos -= forward * camSpeed * deltaTime;

		Vec3f up = toCameraSpace.GetUpVector().GetNormalized();
		if (Input::GetKeyHeld(SDL_SCANCODE_SPACE))
			pTrans->localPos += up * camSpeed * deltaTime;
		if (Input::GetKeyHeld(SDL_SCANCODE_LCTRL))
			pTrans->localPos -= up * camSpeed * deltaTime;

		if (Input::GetMouseInRelativeMode())
		{
			pCam->horizontalAngle += 0.1f * deltaTime * Input::GetMouseDelta().x;
			pCam->verticalAngle += 0.1f * deltaTime * Input::GetMouseDelta().y;
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
	CCube* pCube = scene.Assign<CCube>(cube);
	CTransform* pTrans = scene.Assign<CTransform>(cube);
	pTrans->localPos = Vec3f(0.0f, 0.0f, 3.0f);
	pTrans->localSca = Vec3f(0.5f, 0.5f, 0.5f);
	
	eastl::vector<VertexInputElement> layout;
	layout.push_back({"SV_POSITION",AttributeType::Float3});
	layout.push_back({"COLOR", AttributeType::Float3});

	VertexShaderHandle vShader = GfxDevice::CreateVertexShader(L"Shaders/CubeShader.hlsl", "VSMain", layout, "CubeVertShader");
	PixelShaderHandle pShader = GfxDevice::CreatePixelShader(L"Shaders/CubeShader.hlsl", "PSMain", "CubePixelShader");

	pCube->program = GfxDevice::CreateProgram(vShader, pShader);

	eastl::vector<Vertex> cubeVerts = {
        Vertex(Vec3f(-1.0f, -1.0f,  1.0f)),
        Vertex(Vec3f(1.0f, -1.0f,  1.0f)),
        Vertex(Vec3f(-1.0f,  1.0f,  1.0f)),
        Vertex(Vec3f(1.0f,  1.0f,  1.0f)),

        Vertex(Vec3f(-1.0f, -1.0f, -1.0f)),
        Vertex(Vec3f(1.0f, -1.0f, -1.0f)),
        Vertex(Vec3f(-1.0f,  1.0f, -1.0f)),
        Vertex(Vec3f(1.0f,  1.0f, -1.0f))
    };

	cubeVerts[4].col = Vec3f(1.0f, 0.0f, 0.0f);
	cubeVerts[5].col = Vec3f(0.0f, 1.0f, 0.0f);
	cubeVerts[6].col = Vec3f(1.0f, 0.0f, 1.0f);
	cubeVerts[7].col = Vec3f(1.0f, 1.0f, 1.0f);

	cubeVerts[0].col = Vec3f(1.0f, 0.0f, 0.0f);
	cubeVerts[1].col = Vec3f(0.0f, 1.0f, 0.0f);
	cubeVerts[2].col = Vec3f(1.0f, 0.0f, 1.0f);
	cubeVerts[3].col = Vec3f(1.0f, 1.0f, 1.0f);

	pCube->vBuffer = GfxDevice::CreateVertexBuffer(8, sizeof(Vertex), cubeVerts.data(), "CubeVertBuffer");

	eastl::vector<int> cubeIndices = {
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	};
	pCube->iBuffer = GfxDevice::CreateIndexBuffer(14, cubeIndices.data(), "CubeIndexBuffer");

	pCube->constBuffer = GfxDevice::CreateConstantBuffer(sizeof(cbTransformBuf), "CubeTransBuffer");

	EntityID cube2 = scene.NewEntity("Cube2");
	CCube* pCube2 = scene.Assign<CCube>(cube2);
	scene.Assign<CParent>(cube2)->parent = cube;
	CTransform* pTrans2 = scene.Assign<CTransform>(cube2);
	pTrans2->localPos = Vec3f(1.0f, 0.0f, 5.0f);
	pTrans2->localSca = Vec3f(1.0f, 1.0f, 1.0f);
	*pCube2 = *pCube;
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