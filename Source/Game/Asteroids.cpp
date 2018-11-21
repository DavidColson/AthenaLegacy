#include "Asteroids.h"

#include "Systems/DrawPolygon.h"
#include "Systems/Movement.h"
#include "Components/Components.h"

#include <GameFramework/World.h>

#include <functional>

Space* g_pCurrentSpace;

void Game::Startup()
{
	// Create a player control struct
	CPlayerControl player;
	
	// Retrieve the type of player, and then get the m_pos member from it
	TypeDatabase::Member* posmember = TypeDatabase::GetType(player)->GetMember("m_pos");

	// Get type of m_pos, and then get the "x" member of it
	TypeDatabase::Member* xmember = posmember->m_type->GetMember("x");

	// Get whatever the actual value of the position is (don't care what type it is)
	TypeDatabase::VariantBase position = posmember->GetValue(player);

	// Set the x member to 1337
	xmember->SetValue(position, 1337.0f);

	// Set the m_pos vector to the new vector we made
	posmember->SetValue(player, position);


	Log::Print(Log::EMsg, "------- Serialization Attempt -------");

	

	std::function<void(std::string, std::string, TypeDatabase::RefVariant&&)> stringifyStruct = [&](std::string indent, std::string name, TypeDatabase::RefVariant&& theStruct)
	{
		Log::Print(Log::EMsg, "%s%s %s = {", indent.c_str(), theStruct.m_type->m_name, name.c_str());

		for (std::pair<std::string, TypeDatabase::Member*> member : theStruct.m_type->m_memberList)
		{
			if (member.second->m_type == TypeDatabase::GetType<float>())
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




	// Create our scene
	// ****************
	g_pCurrentSpace = new Space();

	g_pCurrentSpace->RegisterSystem<SDrawPolygon>();
	g_pCurrentSpace->RegisterSystem<SMovement>();

	EntityID asteroid = g_pCurrentSpace->NewEntity();
	g_pCurrentSpace->AssignComponent<CTransform>(asteroid)->m_pos = vec3(100.0f, 100.0f, 0.0f);
	g_pCurrentSpace->AssignComponent<CDrawable>(asteroid)->m_renderProxy = RenderProxy(
		{
			Vertex(vec3(49.6f, 90.6f, 0.5f), color(1.0f, 0.0f, 0.0f)),
			Vertex(vec3(39.f, 51.6f, 0.5f), color(0.0f, 1.0f, 0.0f)),
			Vertex(vec3(19.6f, 54.2f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(5.2f, 31.7f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(24.2f, 8.3f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(51.f, 20.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(97.f, 18.7f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(75.3f, 45.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(95.f, 76.4f, 0.5f), color(0.0f, 0.0f, 1.0f)),
		}, {
			0, 1, 2, 3, 4, 5, 6, 7, 8, 0
		});


	EntityID ship = g_pCurrentSpace->NewEntity();
	CTransform* pTransform = g_pCurrentSpace->AssignComponent<CTransform>(ship);

	pTransform->m_pos = vec3(500.0f, 300.0f, 0.0f);
	pTransform->m_sca = vec3(0.2f, 0.25f, 1.0f);

	g_pCurrentSpace->AssignComponent<CPlayerControl>(ship);
	g_pCurrentSpace->AssignComponent<CDrawable>(ship)->m_renderProxy = RenderProxy(
		{
			Vertex(vec3(0.f, 50.f, 0.5f), color(1.0f, 0.0f, 0.0f)),
			Vertex(vec3(100.f, 80.f, 0.5f), color(0.0f, 1.0f, 0.0f)),
			Vertex(vec3(85.f, 70.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(85.f, 30.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(100.f, 20.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
		}, {
			0, 1, 2, 3, 4, 0
		});

	g_pCurrentSpace->StartSystems();
}

void Game::Update(float deltaTime)
{
	g_pCurrentSpace->UpdateSystems(deltaTime);
}
