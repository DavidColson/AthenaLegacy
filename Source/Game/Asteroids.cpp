#include "Asteroids.h"

#include "Systems/DrawPolygon.h"
#include "Systems/Movement.h"
#include "Components/Components.h"

#include <GameFramework/World.h>

Space* g_pCurrentSpace;

void Game::Startup()
{
	CPlayerControl playerControl;

	// We can now for loop over the properties, printing the name and value of each one. 
	// Without knowing their names
	for (std::pair<std::string, Member_Base*> member : CPlayerControl::GetTypeData()->m_memberList)
	{
		float value = member.second->get_value<float>(&playerControl);
		Log::Print(Log::EMsg, "Member Name: %s Member Value %f", member.first.c_str(), value);
	}

	vec2 myPos = CPlayerControl::GetTypeData()->m_memberList["m_pos"]->get_value<vec2>(&playerControl);
	Log::Print(Log::EMsg, "retrieved vector X: %f Y: %f", myPos.x, myPos.y);

	TypeDatabase::GetTypeData("vec2")->m_memberList["x"]->set_value(&myPos, 22.0f);

	Log::Print(Log::EMsg, "changed vector X: %f Y: %f", myPos.x, myPos.y);

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
