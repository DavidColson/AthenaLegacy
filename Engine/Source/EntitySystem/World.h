#pragma once

#include "EASTL/string.h"
#include "EASTL/vector.h"

class Entity;
class ISystem;

class World
{
public:
	// This will create new element in array and return it to you.
	Entity* NewEntity(eastl::string name);

	// Registers things and turns everything on
	void ActivateWorld();

	void DeactivateWorld();

	// Loops through entities, updating them, then globals
	void OnUpdate(float deltaTime);

    void OnRender(float deltaTime);

	void DestroyWorld();

	template<typename Type>
    ISystem* AddGlobalSystem()
	{
    	globalSystems.push_back(new Type());
		return globalSystems.back();
	}

private:

	eastl::vector<Entity*> entities;
	eastl::vector<ISystem*> globalSystems;
};