#include "Asteroids.h"

#include "Systems/Systems.h"
#include "Components/Components.h"

#include <Editor/Editor.h>
#include <GameFramework/World.h>

#include <functional>
#include <time.h>

namespace {
	Scene* pCurrentScene;
}

void Game::Startup()
{

	// Get the type of CPlayerControl
	TypeDB::Type* playerType = TypeDB::GetTypeFromString("CPlayerControl");

	// Create an instance of the player struct
	TypeDB::Variant player = playerType->New();

	// Get the m_pos member from it
	TypeDB::Member* posmember = playerType->GetMember("m_someVec");

	// Get type of m_pos, and then get the "x" member of it
	TypeDB::Member* xmember = posmember->m_type->GetMember("x");

	// Get whatever the actual value of the position is (don't care what type it is)
	TypeDB::Variant position = posmember->GetValue(player);

	// Set the x member to 1337
	xmember->SetValue(position, 1337.0f);

	// Set the m_pos vector to the new vector we made
	posmember->SetValue(player, position);

	// Get the real value from the player variant
	CPlayerControl realPlayer = player.Get<CPlayerControl>();

	// Prints "realPlayer.m_pos.x 1337.0"
	Log::Print(Log::EMsg, "realPlayer.m_pos.x %f", realPlayer.m_someVec.x);


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

	std::vector<RenderProxy> asteroidMeshes;

	asteroidMeshes.emplace_back(RenderProxy(
		{
			Vertex(vec3(0.03f, 0.379f, 0.0f)),
			Vertex(vec3(0.03f, 0.64f, 0.0f)),
			Vertex(vec3(0.314f, 0.69f, 0.0f)),
			Vertex(vec3(0.348f, 0.96f, 0.0f)),
			Vertex(vec3(0.673f, 0.952f, 0.0f)),
			Vertex(vec3(0.698f, 0.724f, 0.0f)),
			Vertex(vec3(0.97f, 0.645f, 0.0f)),
			Vertex(vec3(0.936f, 0.228f, 0.f)),
			Vertex(vec3(0.555f, 0.028f, 0.f)),
			Vertex(vec3(0.22f, 0.123f, 0.f))
		}, {
			// Note has adjacency data
			9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1
		}));

	asteroidMeshes.emplace_back(RenderProxy(
		{
			Vertex(vec3(0.05f, 0.54f, 0.0f)),
			Vertex(vec3(0.213f, 0.78f, 0.0f)),
			Vertex(vec3(0.37f, 0.65f, 0.0f)),
			Vertex(vec3(0.348f, 0.96f, 0.0f)),
			Vertex(vec3(0.673f, 0.952f, 0.0f)),
			Vertex(vec3(0.64f, 0.75f, 0.0f)),
			Vertex(vec3(0.83f, 0.85f, 0.0f)),
			Vertex(vec3(0.974f, 0.65f, 0.0f)),
			Vertex(vec3(0.943f, 0.298f, 0.f)),
			Vertex(vec3(0.683f, 0.086f, 0.f)),
			Vertex(vec3(0.312f, 0.074f, 0.f)),
			Vertex(vec3(0.056f, 0.265f, 0.f))
		}, {
			// Note has adjacency data
			10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 1
		}));

	asteroidMeshes.emplace_back(RenderProxy(
		{
			Vertex(vec3(0.066f, 0.335f, 0.0f)),
			Vertex(vec3(0.077f, 0.683f, 0.0f)),
			Vertex(vec3(0.3f, 0.762f, 0.0f)),
			Vertex(vec3(0.348f, 0.96f, 0.0f)),
			Vertex(vec3(0.673f, 0.952f, 0.0f)),
			Vertex(vec3(0.724f, 0.752f, 0.0f)),
			Vertex(vec3(0.967f, 0.63f, 0.0f)),
			Vertex(vec3(0.946f, 0.312f, 0.0f)),
			Vertex(vec3(0.706f, 0.353f, 0.f)),
			Vertex(vec3(0.767f, 0.07f, 0.f)),
			Vertex(vec3(0.37f, 0.07f, 0.f)),
			Vertex(vec3(0.21f, 0.33f, 0.f))
		}, {
			// Note has adjacency data
			11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1
		}));

	asteroidMeshes.emplace_back(RenderProxy(
		{
			Vertex(vec3(0.056f, 0.284f, 0.0f)),
			Vertex(vec3(0.064f, 0.752f, 0.0f)),
			Vertex(vec3(0.353f, 0.762f, 0.0f)),
			Vertex(vec3(0.286f, 0.952f, 0.0f)),
			Vertex(vec3(0.72f, 0.944f, 0.0f)),
			Vertex(vec3(0.928f, 0.767f, 0.0f)),
			Vertex(vec3(0.962f, 0.604f, 0.0f)),
			Vertex(vec3(0.568f, 0.501f, 0.0f)),
			Vertex(vec3(0.967f, 0.366f, 0.f)),
			Vertex(vec3(0.857f, 0.16f, 0.f)),
			Vertex(vec3(0.563f, 0.217f, 0.f)),
			Vertex(vec3(0.358f, 0.043f, 0.f))
		}, {
			// Note has adjacency data
			11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1
		}));

	// Create our scene
	// ****************
	pCurrentScene = new Scene();

	srand(uint(time(nullptr)));

	auto randf = []() { return float(rand()) / float(RAND_MAX); };

	for (int i=0; i < 15; i++)
	{
		vec3 randomLocation = vec3(float(rand() % 1800), float(rand() % 1000), 0.0f);
		vec3 randomVelocity = vec3(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
		float randomRotation = randf() * 6.282f;
		EntityID asteroid = pCurrentScene->NewEntity();
		CTransform* pTranform = pCurrentScene->AssignComponent<CTransform>(asteroid);
		pTranform->m_pos = randomLocation;
		pTranform->m_sca = vec3(90.0f, 90.0f, 1.0f);
		pTranform->m_vel = randomVelocity;
		pTranform->m_rot = randomRotation;

		pCurrentScene->AssignComponent<CDrawable>(asteroid)->m_renderProxy = asteroidMeshes[rand() % 4];
	}

	// Ship
	EntityID ship = pCurrentScene->NewEntity();
	CTransform* pTransform = pCurrentScene->AssignComponent<CTransform>(ship);

	pTransform->m_pos = vec3(450.0f, 250.0f, 0.0f);
	pTransform->m_sca = vec3(30.f, 35.f, 1.0f);

	pCurrentScene->AssignComponent<CPlayerControl>(ship);
	pCurrentScene->AssignComponent<CDrawable>(ship)->m_renderProxy = RenderProxy(
		{
			Vertex(vec3(0.f, 0.5f, 0.f)),
			Vertex(vec3(1.f, 0.8f, 0.f)),
			Vertex(vec3(0.9f, 0.7f, 0.f)),
			Vertex(vec3(0.9f, 0.3f, 0.f)),
			Vertex(vec3(1.0f, 0.2f, 0.f)),
		}, {
			// Note, has adjacency data
			4, 0, 1, 2, 3, 4, 0, 1
		});

	Editor::SetCurrentScene(pCurrentScene);
}

void Game::Update(float deltaTime)
{
	ShipControlSystemUpdate(pCurrentScene, deltaTime);
	MovementSystemUpdate(pCurrentScene, deltaTime);
	DrawShapeSystem(pCurrentScene, deltaTime);
}
