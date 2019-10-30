#include "Asteroids.h"
#include "Systems.h"
#include "Components.h"

#include <Maths/Vec4.h>
#include <Maths/Matrix.h>
#include <Editor/Editor.h>
#include <Scene.h>
#include <Engine.h>
#include <IGame.h>
#include <Profiler.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderFont.h>
#include <ThirdParty/Imgui/imgui.h>
#include <TypeSystem.h>

#include <functional>
#include <time.h>

std::vector<RenderProxy> Game::g_asteroidMeshes;
RenderProxy Game::g_shipMesh;

struct Component
{
  int myInt{ 5 };
  int mySecondInt{ 3 };

  REFLECT()
};

REFLECT_BEGIN(Component)
REFLECT_MEMBER(myInt)
REFLECT_MEMBER(mySecondInt)
REFLECT_END()

struct Asteroids : public IGame
{
	void OnStart(Scene& scene) override
	{
		// Move this to type system tests
		Component testComponent;

		TypeData& typeData = TypeDatabase::Get<Component>();

		TypeData& sameTypeData = TypeDatabase::GetFromString("Component");
		TypeData& intTypeData = TypeDatabase::GetFromString("int");

		Member& myInMember = typeData.GetMember("myInt");

		bool isInt = myInMember.IsType<int>();

		myInMember.Set(&testComponent, 1337);

		Log::Print(Log::EMsg, "Iterator printing Members of type: %s", typeData.name);
		for (Member& member : typeData)
		{			
			Log::Print(Log::EMsg, "Name: %s Type: %s val: %i", member.name, member.GetType().name, *member.Get<int>(&testComponent));
		}

		Game::g_asteroidMeshes.emplace_back(RenderProxy(
			{
				Vertex(Vec3f(0.03f, 0.379f, 0.0f)),
				Vertex(Vec3f(0.03f, 0.64f, 0.0f)),
				Vertex(Vec3f(0.314f, 0.69f, 0.0f)),
				Vertex(Vec3f(0.348f, 0.96f, 0.0f)),
				Vertex(Vec3f(0.673f, 0.952f, 0.0f)),
				Vertex(Vec3f(0.698f, 0.724f, 0.0f)),
				Vertex(Vec3f(0.97f, 0.645f, 0.0f)),
				Vertex(Vec3f(0.936f, 0.228f, 0.f)),
				Vertex(Vec3f(0.555f, 0.028f, 0.f)),
				Vertex(Vec3f(0.22f, 0.123f, 0.f))
			}, {
				// Note has adjacency data
				9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1
			}));

		Game::g_asteroidMeshes.emplace_back(RenderProxy(
			{
				Vertex(Vec3f(0.05f, 0.54f, 0.0f)),
				Vertex(Vec3f(0.213f, 0.78f, 0.0f)),
				Vertex(Vec3f(0.37f, 0.65f, 0.0f)),
				Vertex(Vec3f(0.348f, 0.96f, 0.0f)),
				Vertex(Vec3f(0.673f, 0.952f, 0.0f)),
				Vertex(Vec3f(0.64f, 0.75f, 0.0f)),
				Vertex(Vec3f(0.83f, 0.85f, 0.0f)),
				Vertex(Vec3f(0.974f, 0.65f, 0.0f)),
				Vertex(Vec3f(0.943f, 0.298f, 0.f)),
				Vertex(Vec3f(0.683f, 0.086f, 0.f)),
				Vertex(Vec3f(0.312f, 0.074f, 0.f)),
				Vertex(Vec3f(0.056f, 0.265f, 0.f))
			}, {
				// Note has adjacency data
				10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 1
			}));

		Game::g_asteroidMeshes.emplace_back(RenderProxy(
			{
				Vertex(Vec3f(0.066f, 0.335f, 0.0f)),
				Vertex(Vec3f(0.077f, 0.683f, 0.0f)),
				Vertex(Vec3f(0.3f, 0.762f, 0.0f)),
				Vertex(Vec3f(0.348f, 0.96f, 0.0f)),
				Vertex(Vec3f(0.673f, 0.952f, 0.0f)),
				Vertex(Vec3f(0.724f, 0.752f, 0.0f)),
				Vertex(Vec3f(0.967f, 0.63f, 0.0f)),
				Vertex(Vec3f(0.946f, 0.312f, 0.0f)),
				Vertex(Vec3f(0.706f, 0.353f, 0.f)),
				Vertex(Vec3f(0.767f, 0.07f, 0.f)),
				Vertex(Vec3f(0.37f, 0.07f, 0.f)),
				Vertex(Vec3f(0.21f, 0.33f, 0.f))
			}, {
				// Note has adjacency data
				11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1
			}));

		Game::g_asteroidMeshes.emplace_back(RenderProxy(
			{
				Vertex(Vec3f(0.056f, 0.284f, 0.0f)),
				Vertex(Vec3f(0.064f, 0.752f, 0.0f)),
				Vertex(Vec3f(0.353f, 0.762f, 0.0f)),
				Vertex(Vec3f(0.286f, 0.952f, 0.0f)),
				Vertex(Vec3f(0.72f, 0.944f, 0.0f)),
				Vertex(Vec3f(0.928f, 0.767f, 0.0f)),
				Vertex(Vec3f(0.962f, 0.604f, 0.0f)),
				Vertex(Vec3f(0.568f, 0.501f, 0.0f)),
				Vertex(Vec3f(0.967f, 0.366f, 0.f)),
				Vertex(Vec3f(0.857f, 0.16f, 0.f)),
				Vertex(Vec3f(0.563f, 0.217f, 0.f)),
				Vertex(Vec3f(0.358f, 0.043f, 0.f))
			}, {
				// Note has adjacency data
				11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1
			}));

		Game::g_shipMesh = RenderProxy(
			{
				Vertex(Vec3f(0.f, 0.5f, 0.f)),
				Vertex(Vec3f(1.f, 0.8f, 0.f)),
				Vertex(Vec3f(0.9f, 0.7f, 0.f)),
				Vertex(Vec3f(0.9f, 0.3f, 0.f)),
				Vertex(Vec3f(1.0f, 0.2f, 0.f)),
			}, {
				// Note, has adjacency data
				4, 0, 1, 2, 3, 4, 0, 1
			});

		srand(unsigned int(time(nullptr)));
		auto randf = []() { return float(rand()) / float(RAND_MAX); };

		// Create the ship
		EntityID ship = scene.NewEntity("Player Ship");
		ASSERT(ship == PLAYER_ID, "Player must be spawned first");
		CTransform* pTransform = scene.Assign<CTransform>(ship);

		const float w = GfxDevice::GetWindowWidth();
		const float h = GfxDevice::GetWindowHeight();
		pTransform->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
		pTransform->sca = Vec3f(30.f, 35.f, 1.0f);

		CPlayerControl* pPlayer = scene.Assign<CPlayerControl>(ship);
		scene.Assign<CCollidable>(ship)->radius = 17.f;
		scene.Assign<CDrawable>(ship)->renderProxy = Game::g_shipMesh;
		CPlayerUI* pPlayerUI = scene.Assign<CPlayerUI>(ship);
		scene.Assign<CPostProcessing>(ship);

		// Create some asteroids
		for (int i = 0; i < 15; i++)
		{
			Vec3f randomLocation = Vec3f(float(rand() % 1800), float(rand() % 1000), 0.0f);
			Vec3f randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
			float randomRotation = randf() * 6.282f;
			EntityID asteroid = scene.NewEntity("Asteroid");
			scene.Assign<CCollidable>(asteroid);
			CTransform* pTranform = scene.Assign<CTransform>(asteroid);
			pTranform->pos = randomLocation;
			pTranform->sca = Vec3f(90.0f, 90.0f, 1.0f);
			pTranform->vel = randomVelocity;
			pTranform->rot = randomRotation;

			scene.Assign<CDrawable>(asteroid)->renderProxy = Game::g_asteroidMeshes[rand() % 4];
			scene.Assign<CAsteroid>(asteroid);
		}

		// Create the lives
		float offset = 0.0f;
		for (int i = 0; i < 3; ++i)
		{
			EntityID life = scene.NewEntity("Life");
			
			CTransform* pTransform = scene.Assign<CTransform>(life);
			pTransform->pos = Vec3f(150.f + offset, h - 85.0f, 0.0f);
			pTransform->sca = Vec3f(30.f, 35.f, 1.0f);
			pTransform->rot = -3.14159f / 2.0f;
			offset += 30.0f;
			
			scene.Assign<CDrawable>(life)->renderProxy = Game::g_shipMesh;
			pPlayer->lifeEntities[i] = life;
		}


		// Create score counters
		{
			EntityID currentScoreEnt =scene.NewEntity("Current Score");
			pPlayerUI->currentScoreEntity = currentScoreEnt;
			scene.Assign<CText>(currentScoreEnt)->text = "0";
			scene.Assign<CTransform>(currentScoreEnt)->pos = Vec3f(150.0f, h - 53.0f, 0.0f);
			scene.Assign<CPlayerScore>(currentScoreEnt);

			EntityID highScoreEnt = scene.NewEntity("High Score");
			scene.Assign<CText>(highScoreEnt)->text = "0";
			scene.Assign<CTransform>(highScoreEnt)->pos = Vec3f(w - 150.f, h - 53.0f, 0.0f);
		}

		// Create game over text
		{
			EntityID gameOver = scene.NewEntity("Game Over");
			pPlayerUI->gameOverEntity = gameOver;
			scene.Assign<CTransform>(gameOver)->pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
			scene.Assign<CGameOver>(gameOver);
			CText* pText = scene.Assign<CText>(gameOver);
			pText->text = "Game Over";
			pText->visible = false;
		}
	}

	void OnFrame(Scene& scene, float deltaTime) override
	{
		PROFILE();

		ShipControlSystemUpdate(scene, deltaTime);
		MovementSystemUpdate(scene, deltaTime);
		CollisionSystemUpdate(scene, deltaTime);
		InvincibilitySystemUpdate(scene, deltaTime);
	}

	void OnEnd(Scene& scene) override
	{

	}
};

int main(int argc, char *argv[])
{
	Engine::Run(new Asteroids(), new Scene());
	return 0;
}