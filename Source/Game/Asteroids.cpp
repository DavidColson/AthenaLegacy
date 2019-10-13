#include "TypeData.h"


#include "TypeDB.h"
#include "Asteroids.h"
#include "Systems/Systems.h"
#include "Components/Components.h"

#include <Maths/Vec4.h>
#include <Maths/Matrix.h>
#include <Editor/Editor.h>
#include <GameFramework/World.h>
#include <Engine.h>
#include <IGame.h>
#include <Renderer/Renderer.h>
#include <ThirdParty/Imgui/imgui.h>
#include <TypeSystem.h>

#include <functional>
#include <time.h>

namespace {
	Scene* pCurrentScene;
}

std::vector<RenderProxy> Game::g_asteroidMeshes;
RenderProxy Game::g_shipMesh;

struct Asteroids : public IGame
{
	void OnStart() override
	{



		Component testComponent;

		TypeData* typeData = TypeDatabase::Get<Component>();

		TypeData* sameTypeData = TypeDatabase::GetFromString("Component");
		TypeData* intTypeData = TypeDatabase::GetFromString("int");


		Member* myIntMember = typeData->GetMember("myInt");
		myIntMember->Set(&testComponent, 1337);

		Log::Print(Log::EMsg, "Printing Members of type: %s", typeData->m_name);
		for (std::pair<std::string, Member> member : typeData->m_members)
		{
			Log::Print(Log::EMsg, "Name: %s Type: %s val: %i", member.first.c_str(), member.second.m_type->m_name, *member.second.Get<int>(&testComponent));
		}




		// Get the type of CPlayerControl
		TypeDB::Type* playerType = TypeDB::GetTypeFromString("CPlayerControl");

		// Create an instance of the player struct
		TypeDB::Variant player = playerType->New();

		// Get the real value from the player variant
		CPlayerControl realPlayer = player.Get<CPlayerControl>();

		Log::Print(Log::EMsg, "------- Serialization Attempt -------");

		std::function<void(std::string, std::string, TypeDB::RefVariant&&)> stringifyStruct = [&](std::string indent, std::string name, TypeDB::RefVariant&& theStruct)
		{
			Log::Print(Log::EMsg, "%s%s %s = {", indent.c_str(), theStruct.m_type->m_name, name.c_str());

			for (std::pair<std::string, TypeDB::Member*> member : theStruct.m_type->m_memberList)
			{
				if (member.second->m_type == TypeDB::GetType<float>())
				{
					float value = member.second->GetValue(theStruct).Get<float>();
					Log::Print(Log::EMsg, "%s    %s %s = %s", indent.c_str(), member.second->m_type->m_name, member.first.c_str(), std::to_string(value).c_str());
				}
				else
				{
					stringifyStruct(indent + "    ", member.first, member.second->GetValue(theStruct));
				}
			}
			Log::Print(Log::EMsg, "%s}", indent.c_str());
		};

		stringifyStruct("", "player", player);

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

		// Create our scene
		// ****************
		pCurrentScene = new Scene();

		srand(unsigned int(time(nullptr)));

		auto randf = []() { return float(rand()) / float(RAND_MAX); };

		// Create some asteroids
		for (int i = 0; i < 15; i++)
		{
			Vec3f randomLocation = Vec3f(float(rand() % 1800), float(rand() % 1000), 0.0f);
			Vec3f randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
			float randomRotation = randf() * 6.282f;
			EntityID asteroid = pCurrentScene->NewEntity("Asteroid");
			pCurrentScene->Assign<CCollidable>(asteroid);
			CTransform* pTranform = pCurrentScene->Assign<CTransform>(asteroid);
			pTranform->m_pos = randomLocation;
			pTranform->m_sca = Vec3f(90.0f, 90.0f, 1.0f);
			pTranform->m_vel = randomVelocity;
			pTranform->m_rot = randomRotation;

			pCurrentScene->Assign<CDrawable>(asteroid)->m_renderProxy = Game::g_asteroidMeshes[rand() % 4];
			pCurrentScene->Assign<CAsteroid>(asteroid);
		}

		// Create the ship
		EntityID ship = pCurrentScene->NewEntity("Player Ship");
		CTransform* pTransform = pCurrentScene->Assign<CTransform>(ship);

		const float w = Graphics::GetContext()->m_windowWidth;
		const float h = Graphics::GetContext()->m_windowHeight;
		pTransform->m_pos = Vec3f(w/2.0f, h/2.0f, 0.0f);
		pTransform->m_sca = Vec3f(30.f, 35.f, 1.0f);

		CPlayerControl* pPlayer = pCurrentScene->Assign<CPlayerControl>(ship);
		pCurrentScene->Assign<CCollidable>(ship)->m_radius = 17.f;
		pCurrentScene->Assign<CDrawable>(ship)->m_renderProxy = Game::g_shipMesh;

		// Create the lives
		float offset = 0.0f;
		for (int i = 0; i < 3; ++i)
		{
			EntityID life = pCurrentScene->NewEntity("Life");
			
			CTransform* pTransform = pCurrentScene->Assign<CTransform>(life);
			pTransform->m_pos = Vec3f(150.f + offset, h - 85.0f, 0.0f);
			pTransform->m_sca = Vec3f(30.f, 35.f, 1.0f);
			pTransform->m_rot = -3.14159f / 2.0f;
			offset += 30.0f;
			
			pCurrentScene->Assign<CDrawable>(life)->m_renderProxy = Game::g_shipMesh;
			pPlayer->m_lifeEntities[i] = life;
		}


		// Create score counters
		{
			EntityID currentScoreEnt = pCurrentScene->NewEntity("Current Score");
			pCurrentScene->Assign<CText>(currentScoreEnt)->m_text = "0";
			pCurrentScene->Assign<CTransform>(currentScoreEnt)->m_pos = Vec3f(150.0f, h - 53.0f, 0.0f);
			pCurrentScene->Assign<CPlayerScore>(currentScoreEnt);

			EntityID highScoreEnt = pCurrentScene->NewEntity("High Score");
			pCurrentScene->Assign<CText>(highScoreEnt)->m_text = "0";
			pCurrentScene->Assign<CTransform>(highScoreEnt)->m_pos = Vec3f(w - 150.f, h - 53.0f, 0.0f);
		}

		// Create game over text
		{
			EntityID gameOver = pCurrentScene->NewEntity("Game Over");
			pCurrentScene->Assign<CTransform>(gameOver)->m_pos = Vec3f(w / 2.0f, h / 2.0f, 0.0f);
			pCurrentScene->Assign<CGameOver>(gameOver);
			CText* pText = pCurrentScene->Assign<CText>(gameOver);
			pText->m_text = "Game Over";
			pText->m_visible = false;
		}

		Editor::SetCurrentScene(pCurrentScene);
	}

	void OnFrame(float deltaTime) override
	{
		//Log::Print(Log::EMsg, "----- Frame Start -----");

		// #RefactorNote: Make systems into more carefully managed classes, and tidy up this performance profiling.
		// Make into a more fleshed out imgui window
		Uint64 start = SDL_GetPerformanceCounter();
		ShipControlSystemUpdate(pCurrentScene, deltaTime);
		float shipControl = float(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();

		start = SDL_GetPerformanceCounter();
		MovementSystemUpdate(pCurrentScene, deltaTime);
		float movement = float(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();

		start = SDL_GetPerformanceCounter();
		CollisionSystemUpdate(pCurrentScene, deltaTime);
		float collision = float(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
		
		start = SDL_GetPerformanceCounter();
		DrawShapeSystem(pCurrentScene, deltaTime);
		float drawShape = float(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
		
		start = SDL_GetPerformanceCounter();
		DrawTextSystem(pCurrentScene, deltaTime);
		float drawText = float(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
	
		ImGui::Begin("Profiler");

		ImGui::Text(StringFormat("Ship Control System %f", shipControl).c_str());
		ImGui::Text(StringFormat("Movement System %f", movement).c_str());
		ImGui::Text(StringFormat("Collision System %f", collision).c_str());
		ImGui::Text(StringFormat("Draw Shape System %f", drawShape).c_str());
		ImGui::Text(StringFormat("Draw Text System %f", drawText).c_str());

		ImGui::End();
	}

	void OnEnd() override
	{

	}
};

int main(int argc, char *argv[])
{
	Asteroids* pAsteroids = new Asteroids;

	Engine::Startup(pAsteroids);
	Engine::Run();
	Engine::Shutdown();
	
	return 0;
}