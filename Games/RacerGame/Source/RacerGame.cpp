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

	GfxDraw::Line(Vec3f(3.0f, 2.0f, 0.0f), Vec3f(3.2f, 4.0f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), 0.01f);
	GfxDraw::Line(Vec3f(3.3f, 2.0f, 0.0f), Vec3f(3.5f, 4.0f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), 0.05f);
	GfxDraw::Line(Vec3f(3.6f, 2.0f, 0.0f), Vec3f(3.8f, 4.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), 0.1f);

	GfxDraw::PolylineShape poly;

	poly.closed = true;
	poly.AddPoint(Vec3f(1.0f, -1.0f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), 0.1f);
	poly.AddPoint(Vec3f(-1.0f, -1.0f, 0.0f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), 0.1f);
	poly.AddPoint(Vec3f(-2.0f, 0.0f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), 0.1f);
	poly.AddPoint(Vec3f(-1.0f, 1.0f, 0.0f), Vec4f(1.0f, 0.0f, 1.0f, 1.0f), 0.1f);
	poly.AddPoint(Vec3f(0.0f, 0.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), 0.1f);
	poly.AddPoint(Vec3f(1.0f, 1.0f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), 0.1f);
	poly.GenerateMesh();

	GfxDraw::Polyline(poly);

	GfxDraw::Rect(Vec3f(-4.0f, -2.0f, 0.0f), Vec2f(4.0f, 2.0f), Vec4f(0.6f, 0.0f, 0.0f, 1.0f), Vec4f(0.3f), 0.1f);
	GfxDraw::Rect(Vec3f(-4.0f, 2.0f, 0.0f), Vec2f(1.0f, 4.0f), Vec4f(0.0f, 0.7f, 0.0f, 1.0f), Vec4f(0.5f, 0.0f, 0.0f, 0.2f), 0.2f);
	GfxDraw::Rect(Vec3f(-4.0f, 6.0f, 0.0f), Vec2f(2.0f, 2.8f), Vec4f(1.0f), Vec4f(0.0f), 0.1f);

	float pi = 3.1415926f;

	GfxDraw::Circle(Vec3f(0.0f, 3.0f, -0.01f), 1.2f, Vec4f(1.0f));

	GfxDraw::Pie(Vec3f(3.0f, 0.0f, 0.0f), 0.8f, 0.12f, pi * 1.6f, Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
	
	GfxDraw::Ring(Vec3f(0.0f, 3.0f, 0.0f), 1.5f, 0.1f, Vec4f(0.0f, 1.0f, 0.0f, 1.0f));

	GfxDraw::Arc(Vec3f(0.0f, -3.0f, 0.0f), 1.5f, 0.4f, 0.3f, pi, Vec4f(0.0f, 0.0f, 1.0f, 1.0f));

	GfxDraw::PolygonShape asteroidshape;
	
	asteroidshape.AddPoint(Vec2f(0.056f, 0.265f));
	asteroidshape.AddPoint(Vec2f(0.312f, 0.074f));
	asteroidshape.AddPoint(Vec2f(0.683f, 0.086f));
	asteroidshape.AddPoint(Vec2f(0.943f, 0.298f));
	asteroidshape.AddPoint(Vec2f(0.974f, 0.65f));
	asteroidshape.AddPoint(Vec2f(0.83f, 0.85f));
	asteroidshape.AddPoint(Vec2f(0.64f, 0.75f));
	asteroidshape.AddPoint(Vec2f(0.673f, 0.952f));
	asteroidshape.AddPoint(Vec2f(0.348f, 0.96f));
	asteroidshape.AddPoint(Vec2f(0.37f, 0.65f));
	asteroidshape.AddPoint(Vec2f(0.213f, 0.78f));
	asteroidshape.AddPoint(Vec2f(0.05f, 0.54f));
	for (size_t i = 0; i < asteroidshape.points.size(); i++)
	{
		asteroidshape.points[i] *= 2.0f;
		asteroidshape.points[i] += Vec2f(-1.0f, 5.0f);
	}
	asteroidshape.GenerateMesh();

	GfxDraw::Polygon(asteroidshape);

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
	// JsonValue jsonScene = ParseJsonFile(FileSys::ReadWholeFile("Games/RacerGame/Resources/Levels/RacerGame.lvl"));
	// Scene* pScene = SceneSerializer::NewSceneFromJson(jsonScene);

	Scene* pScene = new Scene();

	// Register systems
	Engine::SetSceneCreateCallback([](Scene& newScene) {
		newScene.RegisterSystem(SystemPhase::Update, CameraControlSystem);
	});

	// Run everything
	Engine::Run(pScene);

	return 0;
}