#include "TypeData.h"


#include "TypeDB.h"
#include "Systems/Systems.h"
#include "Components/Components.h"

#include <Maths/Vec4.h>
#include <Maths/Matrix.h>
#include <Editor/Editor.h>
#include <GameFramework/World.h>
#include <Engine.h>
#include <IGame.h>

#include <functional>
#include <time.h>

namespace {
	Scene* pCurrentScene;
}

std::vector<RenderProxy> g_asteroidMeshes;

struct Asteroids : public IGame
{
	void OnStart() override
	{
		// Get the type of CPlayerControl
		TypeDB::Type* playerType = TypeDB::GetTypeFromString("CPlayerControl");

		// Create an instance of the player struct
		TypeDB::Variant player = playerType->New();

		// Get the m_pos member from it
		TypeDB::Member* posmember = playerType->GetMember("m_newVec");

		// Get type of m_pos, and then get the "x" member of it
		TypeDB::Member* xmember = posmember->GetType()->GetMember("x");

		// Get whatever the actual value of the position is (don't care what type it is)
		TypeDB::Variant position = posmember->GetValue(player);

		// Set the x member to 1337
		xmember->SetValue(position, 1337.0f);

		// Set the m_pos vector to the new vector we made
		posmember->SetValue(player, position);

		// Get the real value from the player variant
		CPlayerControl realPlayer = player.Get<CPlayerControl>();

		// Prints "realPlayer.m_pos.x 1337.0"
		Log::Print(Log::EMsg, "realPlayer.m_pos.x %f", realPlayer.m_newVec.x);


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

		g_asteroidMeshes.emplace_back(RenderProxy(
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

		g_asteroidMeshes.emplace_back(RenderProxy(
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

		g_asteroidMeshes.emplace_back(RenderProxy(
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

		g_asteroidMeshes.emplace_back(RenderProxy(
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

		// Create our scene
		// ****************
		pCurrentScene = new Scene();

		srand(unsigned int(time(nullptr)));

		auto randf = []() { return float(rand()) / float(RAND_MAX); };

		for (int i = 0; i < 15; i++)
		{
			Vec3f randomLocation = Vec3f(float(rand() % 1800), float(rand() % 1000), 0.0f);
			Vec3f randomVelocity = Vec3f(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, 0.0f)  * 40.0f;
			float randomRotation = randf() * 6.282f;
			EntityID asteroid = pCurrentScene->NewEntity();
			pCurrentScene->AssignComponent<CCollidable>(asteroid);
			CTransform* pTranform = pCurrentScene->AssignComponent<CTransform>(asteroid);
			pTranform->m_pos = randomLocation;
			pTranform->m_sca = Vec3f(90.0f, 90.0f, 1.0f);
			pTranform->m_vel = randomVelocity;
			pTranform->m_rot = randomRotation;

			pCurrentScene->AssignComponent<CDrawable>(asteroid)->m_renderProxy = g_asteroidMeshes[rand() % 4];
			pCurrentScene->AssignComponent<CAsteroid>(asteroid);
		}

		// Ship
		EntityID ship = pCurrentScene->NewEntity();
		CTransform* pTransform = pCurrentScene->AssignComponent<CTransform>(ship);

		pTransform->m_pos = Vec3f(450.0f, 250.0f, 0.0f);
		pTransform->m_sca = Vec3f(30.f, 35.f, 1.0f);

		pCurrentScene->AssignComponent<CPlayerControl>(ship);
		pCurrentScene->AssignComponent<CCollidable>(ship)->m_radius = 17.f;
		pCurrentScene->AssignComponent<CDrawable>(ship)->m_renderProxy = RenderProxy(
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

		Editor::SetCurrentScene(pCurrentScene);
	}

	void OnFrame(float deltaTime) override
	{
		ShipControlSystemUpdate(pCurrentScene, deltaTime);
		MovementSystemUpdate(pCurrentScene, deltaTime);
		CollisionSystemUpdate(pCurrentScene, deltaTime);
		AsteroidSystemUpdate(pCurrentScene, deltaTime);
		DrawShapeSystem(pCurrentScene, deltaTime);
	}

	void OnEnd() override
	{

	}
};

int main(int argc, char *argv[])
{
	Asteroids* pAsteroids = new Asteroids;


	Matrixf mat;
	mat.m[0][0] = 1.0f; mat.m[0][1] = 2.0f; mat.m[0][2] = 3.0f; mat.m[0][3] = 4.0f;
	mat.m[1][0] = 2.0f; mat.m[1][1] = 4.0f; mat.m[1][2] = 1.0f; mat.m[1][3] = 2.0f;
	mat.m[2][0] = 5.0f; mat.m[2][1] = 1.0f; mat.m[2][2] = 2.0f; mat.m[2][3] = 3.0f;
	mat.m[3][0] = 3.0f; mat.m[3][1] = 3.0f; mat.m[3][2] = 1.0f; mat.m[3][3] = 1.0f;

	Matrixf mat2;
	mat2.m[0][0] = 4.0f; mat2.m[0][1] = 2.0f; mat2.m[0][2] = 2.0f; mat2.m[0][3] = 4.0f;
	mat2.m[1][0] = 2.0f; mat2.m[1][1] = 1.0f; mat2.m[1][2] = 1.0f; mat2.m[1][3] = 3.0f;
	mat2.m[2][0] = 7.0f; mat2.m[2][1] = 2.0f; mat2.m[2][2] = 2.0f; mat2.m[2][3] = 3.0f;
	mat2.m[3][0] = 5.0f; mat2.m[3][1] = 3.0f; mat2.m[3][2] = 6.0f; mat2.m[3][3] = 1.0f;

	Matrixf result = mat2 * mat;

	Engine::Startup(pAsteroids);
	Engine::Run();
	Engine::Shutdown();
	
	return 0;
}