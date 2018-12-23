#include "Catch/catch.hpp"
#include <GameFramework/World.h>

struct CSomeComponent
{
	float number = 5.0f;
	bool boolean = true;
};

TEST_CASE("ECS Entities And Components")
{
	Scene* pScene = new Scene();

	// Create entities
	EntityID entity = pScene->NewEntity();

	// assign a compoenent to an entity
	pScene->AssignComponent<CSomeComponent>(entity);
	REQUIRE(pScene->HasComponent<CSomeComponent>(entity));
	REQUIRE(pScene->GetComponent<CSomeComponent>(entity)->number == 5.0f);

	// Destroy an entity, component lookups will fail
	pScene->DestroyEntity(entity);
	REQUIRE(pScene->HasComponent<CSomeComponent>(entity) == false);
	REQUIRE(pScene->GetComponent<CSomeComponent>(entity) == nullptr);

	// Make a new entity, it'll share the same index as the last one
	EntityID entity2 = pScene->NewEntity();
	REQUIRE(entity2 != entity);
	REQUIRE(GetEntityIndex(entity) == GetEntityIndex(entity2));
}

struct CComponentOne
{
	float num = 2.0f;
};

struct CComponentTwo
{
	int leet = 1337;
};

TEST_CASE("ECS Scene Views")
{
	Scene* pScene = new Scene();

	// Create entities and give them components
	EntityID entity = pScene->NewEntity();
	pScene->AssignComponent<CComponentOne>(entity);
	pScene->AssignComponent<CComponentTwo>(entity);

	EntityID entity2 = pScene->NewEntity();
	pScene->AssignComponent<CComponentTwo>(entity2);

	EntityID entity3 = pScene->NewEntity();
	pScene->AssignComponent<CComponentOne>(entity3);

	// Loop through all entities with ComponentOne
	for (EntityID ent : SceneView<CComponentOne>(pScene))
	{
		REQUIRE(pScene->HasComponent<CComponentOne>(ent));
		REQUIRE(pScene->GetComponent<CComponentOne>(ent)->num == 2.0f);
	}

	// Loop through all entities with ComponentTwo
	for (EntityID ent : SceneView<CComponentTwo>(pScene))
	{
		REQUIRE(pScene->HasComponent<CComponentTwo>(ent));
		REQUIRE(pScene->GetComponent<CComponentTwo>(ent)->leet == 1337);
	}

	// Loop through all entities with both components
	for (EntityID ent : SceneView<CComponentOne, CComponentTwo>(pScene))
	{
		REQUIRE(ent == entity);
	}
}

