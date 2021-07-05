#pragma once

#include "EASTL/string.h"
#include "EASTL/vector.h"
#include "UUID.h"

class Entity;
class IWorldSystem;
struct UpdateContext;

class World
{
public:
	~World();

	// This will create new element in array and return it to you.
	Entity* NewEntity(eastl::string name);

	void DestroyEntity(Uuid entityId);

	// Registers things and turns everything on
	void ActivateWorld();

	void DeactivateWorld();

	// Loops through entities, updating them, then globals
	void OnUpdate(UpdateContext& ctx);

	template<typename Type>
    IWorldSystem* AddGlobalSystem()
	{
    	globalSystems.push_back(new Type());
		return globalSystems.back();
	}

	/**
	 * Defines a forward iterator on the entities in this world
	 * 
	 * Use like: for(Entity* pEnt : world) { ... }
	 * 
	 **/
	struct EntityIterator
	{
		EntityIterator(eastl::vector<Entity*>::iterator _it) : it(_it) {}

		Entity* operator*() const;

		bool operator==(const EntityIterator& other) const;

		bool operator!=(const EntityIterator& other) const;

		EntityIterator& operator++();

		eastl::vector<Entity*>::iterator it;
	};

	/**
	 * Iterator to first member
	 **/
	const EntityIterator begin();

	/**
	 * Iterator to last member
	 **/
	const EntityIterator end();

private:
	bool isActive{ false };

	eastl::vector<Entity*> entitiesToAddQueue;
	eastl::vector<Entity*> entitiesToDeleteQueue;

	eastl::vector<Entity*> entities;
	eastl::vector<IWorldSystem*> globalSystems;
};