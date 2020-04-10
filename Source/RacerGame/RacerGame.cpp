#include "RacerGame.h"
#include "Systems.h"
#include "Components.h"

#include <Matrix.h>
#include <SDL.h>

struct cbTransformBuf
{
	Matrixf wvp;
};

void CubeRenderSystem(Scene& scene, float deltaTime)
{
	for (EntityID ent : SceneView<CCube>(scene))
    {
		CCube* pCube = scene.Get<CCube>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

		pTrans->rot.z += deltaTime;
		Matrixf translate = Matrixf::Translate(pTrans->pos);
		Matrixf scale = Matrixf::Scale(pTrans->sca);
		Matrixf rotate = Matrixf::Rotate(pTrans->rot);

		Matrixf projection = Matrixf::Perspective(GfxDevice::GetWindowWidth(), GfxDevice::GetWindowHeight(), 0.1f, 100.0f, 60.0f);

		Matrixf wvp = projection * translate * rotate * scale;

		cbTransformBuf trans{ wvp };
		GfxDevice::BindConstantBuffer(pCube->constBuffer, &trans, ShaderType::Vertex, 0);

		GfxDevice::BindProgram(pCube->program);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		GfxDevice::BindVertexBuffers(1, &pCube->vBuffer);
		GfxDevice::BindIndexBuffer(pCube->iBuffer);

		GfxDevice::DrawIndexed(14, 0, 0);
	}
}

void SetupScene(Scene& scene)
{
	scene.RegisterSystem(SystemPhase::Render, CubeRenderSystem);

	EntityID cube = scene.NewEntity("Cube");
	CCube* pCube = scene.Assign<CCube>(cube);
	CTransform* pTrans = scene.Assign<CTransform>(cube);
	pTrans->pos = Vec3f(0.0f, 0.0f, 3.0f);
	pTrans->sca = Vec3f(0.5f, 0.5f, 0.5f);
	
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