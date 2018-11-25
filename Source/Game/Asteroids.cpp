#include "Asteroids.h"

#include "Systems/DrawPolygon.h"
#include "Systems/Movement.h"
#include "Components/Components.h"

#include <Editor/Editor.h>
#include <GameFramework/World.h>

#include <functional>

namespace {
	Space* pCurrentSpace;
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




	// Create our scene
	// ****************
	pCurrentSpace = new Space();

	pCurrentSpace->RegisterSystem<SDrawPolygon>();
	pCurrentSpace->RegisterSystem<SMovement>();

	EntityID asteroid = pCurrentSpace->NewEntity();
	pCurrentSpace->AssignComponent<CTransform>(asteroid)->m_pos = vec3(100.0f, 100.0f, 0.0f);
	pCurrentSpace->AssignComponent<CDrawable>(asteroid)->m_renderProxy = RenderProxy(
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


	EntityID ship = pCurrentSpace->NewEntity();
	CTransform* pTransform = pCurrentSpace->AssignComponent<CTransform>(ship);

	pTransform->m_pos = vec3(500.0f, 300.0f, 0.0f);
	pTransform->m_sca = vec3(0.2f, 0.25f, 1.0f);

	pCurrentSpace->AssignComponent<CPlayerControl>(ship);
	pCurrentSpace->AssignComponent<CDrawable>(ship)->m_renderProxy = RenderProxy(
		{
			Vertex(vec3(0.f, 50.f, 0.5f), color(1.0f, 0.0f, 0.0f)),
			Vertex(vec3(100.f, 80.f, 0.5f), color(0.0f, 1.0f, 0.0f)),
			Vertex(vec3(85.f, 70.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(85.f, 30.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
			Vertex(vec3(100.f, 20.f, 0.5f), color(0.0f, 0.0f, 1.0f)),
		}, {
			0, 1, 2, 3, 4, 0
		});

	pCurrentSpace->StartSystems();
	Editor::SetCurrentSpace(pCurrentSpace);
}

void Game::Update(float deltaTime)
{
	pCurrentSpace->UpdateSystems(deltaTime);
}
