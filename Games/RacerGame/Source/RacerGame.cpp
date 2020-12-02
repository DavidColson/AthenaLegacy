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

namespace
{
	GfxDraw::PolyshapeMesh asteroidPolyShape;
}

void CameraControlSystem(Scene& scene, float deltaTime)
{
	PROFILE();

	GfxDraw::Paint paint;
	paint.strokeThickness = 0.01f;
	paint.strokeColor = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
	GfxDraw::Line(Vec3f(3.0f, 2.0f, 0.0f), Vec3f(3.2f, 4.0f, 0.0f), paint);
	paint.strokeThickness = 0.05f;
	paint.strokeColor = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
	GfxDraw::Line(Vec3f(3.3f, 2.0f, 0.0f), Vec3f(3.5f, 4.0f, 0.0f), paint);
	paint.strokeThickness = 0.1f;
	paint.strokeColor = Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
	GfxDraw::Line(Vec3f(3.6f, 2.0f, 0.0f), Vec3f(3.8f, 4.0f, 0.0f), paint);

	{
		eastl::vector<Vec2f> polyline;
		polyline.push_back(Vec2f(1.0f, 1.0f));
		polyline.push_back(Vec2f(0.0f, 0.0f));
		polyline.push_back(Vec2f(-1.0f, 1.0f));
		polyline.push_back(Vec2f(-2.0f, 0.0f));
		polyline.push_back(Vec2f(-1.0f, -1.0f));
		polyline.push_back(Vec2f(1.0f, -1.0f));
		GfxDraw::Paint polyPaint;
		polyPaint.drawStyle = GfxDraw::DrawStyle::Both;
		polyPaint.fillColor = Vec4f(0.0f, 0.5f, 0.5f, 1.0f);
		polyPaint.strokeThickness = 0.1f;
		polyPaint.strokeColor = Vec4f(1.0f);
		GfxDraw::Polyshape(polyline, polyPaint);
	}

	GfxDraw::Paint rectPaint;
	rectPaint.drawStyle = GfxDraw::DrawStyle::Both;
	rectPaint.fillColor = Vec4f(0.6f, 0.0f, 0.0f, 1.0f);
	rectPaint.strokeColor = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
	rectPaint.strokeThickness = 0.2f;
	GfxDraw::Rect(Vec3f(-4.0f, -2.0f, 0.0f), Vec2f(4.0f, 2.0f), Vec4f(0.6f, 0.0f, 0.0f, 1.0f), rectPaint);

	GfxDraw::Paint circlePaint;
	circlePaint.strokeThickness = 0.1f;
	circlePaint.strokeColor = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
	circlePaint.fillColor = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
	circlePaint.drawStyle = GfxDraw::DrawStyle::Both;
	GfxDraw::Circle(Vec3f(0.0f, 3.0f, -0.01f), 1.2f, circlePaint);
	GfxDraw::Sector(Vec3f(0.0f, -3.0f, -0.01f), 1.2f, 0.1f, 2.0f, circlePaint);

	GfxDraw::Polyshape(asteroidPolyShape);

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


	{
		eastl::vector<Vec2f> asteroidPoly;
		asteroidPoly.push_back(Vec2f(0.056f, 0.265f));
		asteroidPoly.push_back(Vec2f(0.312f, 0.074f));
		asteroidPoly.push_back(Vec2f(0.683f, 0.086f));
		asteroidPoly.push_back(Vec2f(0.943f, 0.298f));
		asteroidPoly.push_back(Vec2f(0.974f, 0.65f));
		asteroidPoly.push_back(Vec2f(0.83f, 0.85f));
		asteroidPoly.push_back(Vec2f(0.64f, 0.75f));
		asteroidPoly.push_back(Vec2f(0.673f, 0.952f));
		asteroidPoly.push_back(Vec2f(0.348f, 0.96f));
		asteroidPoly.push_back(Vec2f(0.37f, 0.65f));
		asteroidPoly.push_back(Vec2f(0.213f, 0.78f));
		asteroidPoly.push_back(Vec2f(0.05f, 0.54f));
		for (size_t i = 0; i < asteroidPoly.size(); i++)
		{
			asteroidPoly[i] *= 2.0f;
			asteroidPoly[i] += Vec2f(-1.0f, 5.0f);
		}
		GfxDraw::Paint asteroidPaint;
		asteroidPaint.drawStyle = GfxDraw::DrawStyle::Stroke;
		asteroidPaint.strokeThickness = 0.1f;
		asteroidPaint.strokeColor = Vec4f(1.0f);
		asteroidPaint.fillColor = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
		asteroidPolyShape = GfxDraw::CreatePolyshape(asteroidPoly, asteroidPaint);
	}

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