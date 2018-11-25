#pragma once

/* EXAMPLE USAGE
/ **************

// Create new components like this

struct Transform
{
	vec3 pos;
};

struct Shape
{
	vec3 color;
};

// Define systems like this:

class MovementSystem : public System
{
public:

	virtual void UpdateEntity(EntityID id) override
	{
		Transform* pTransform = gGameWorld.GetComponent<Transform>(id);

		pTransform->pos.x += 0.01f;
	}

	virtual void SetSubscriptions() override
	{
		Subscribe<Transform>();
	}
};

// You will need to register the system with the game world like this:

gGameWorld.RegisterSystem<MovementSystem>();

// To create entities and assign entities to them do this:

EntityID triangle = gGameWorld.NewEntity();
Transform* pTransform = gGameWorld.AssignComponent<Transform>(triangle);
Shape* pShape = gGameWorld.AssignComponent<Shape>(triangle);

EntityID circle = gGameWorld.NewEntity();
gGameWorld.AssignComponent<Shape>(circle);

// To get the whole thing running, just call gGameWorld.UpdateSystems();

*/

// TODO: Move some of this to a cpp file

#include "Reflection.h"

#include <bitset>
#include <vector>
#include <cassert>

typedef unsigned int EntityID;
const int MAX_COMPONENTS = 10;
const int MAX_ENTITIES = 64;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

struct Space;


// Gives you the id within this world for a given component type
static int s_componentCounter = 0;
template <class T>
int GetComponentId()
{
	// static variable will be initialized on first function call
	// It will then continue to return the same thing, no matter how many times this is called.
	// Allows us to assign a unique id to each component type, since each component type has it's own instance of this function
	static int s_componentId = s_componentCounter++;
	return s_componentId;
	return s_componentId;
}

// Used to relate components to the Type objects in the reflection database
struct ComponentIdToTypeIdMap
{
	void AddRelation(TypeId typeId, int componentId)
	{
		m_typeToComponent[typeId] = componentId;
		m_componentToType[componentId] = typeId;
	}
	int LookupComponentId(TypeId typeId) { return m_typeToComponent[typeId]; }
	TypeId LookupTypeId(int componentId) { return m_componentToType[componentId]; }
private:
	std::unordered_map<TypeId, int> m_typeToComponent;
	std::unordered_map<int, TypeId> m_componentToType;
};
extern ComponentIdToTypeIdMap g_componentTypeMap;

// *****************************************
// Base class for systems
// *****************************************

class System
{
	friend struct Space;
public:

	virtual void StartEntity(EntityID id, Space* space) {};
	virtual void UpdateEntity(EntityID id, Space* space, float deltaTime) = 0;
	virtual void SetSubscriptions() = 0;

protected:
	template <typename T>
	void Subscribe()
	{
		m_componentSubscription.set(GetComponentId<T>());
	}

private:
	ComponentMask m_componentSubscription;
};

// ********************************************
// All components are stored in component pools
// Memory is managed manually
// TODO: Generational indices for reuse of slots
// ********************************************

struct ComponentPool
{
	ComponentPool(size_t elementsize)
	{
		elementSize = elementsize;
		pData = new char[elementSize * MAX_ENTITIES];
		size = MAX_ENTITIES;
	}

	inline void* get(size_t index)
	{
		return pData + index * elementSize;
	}

	char* pData;
	size_t elementSize;
	size_t size = 0;
};

// *****************************************
// A game space, holds enties and systems
// Updates systems and manages creation and
// deletion of entities and components
// You should be able to maintain multiple spaces at once
// *****************************************

struct Space
{
	// TODO Destructor, delete systems and components

	// Goes through each system looping entities and calling the startup function
	// for entities matching the subscription. Called once on scene laod
	void StartSystems()
	{
		for (System* pSys : m_systems)
		{
			for (EntityID i = 0; i < m_entities.size(); i++)
			{
				ComponentMask mask = m_entities[i];
				if (pSys->m_componentSubscription == (pSys->m_componentSubscription & mask))
				{
					pSys->StartEntity(i, this);
				}
			}
		}
	}

	// Goes through each system one by one, looping through all entities and updating the
	// system with entities that match the subscription
	void UpdateSystems(float deltaTime)
	{
		for (System* sys : m_systems)
		{
			for (EntityID i = 0; i < m_entities.size(); i++)
			{
				ComponentMask mask = m_entities[i];
				if (sys->m_componentSubscription == (sys->m_componentSubscription & mask))
				{
					sys->UpdateEntity(i, this, deltaTime);
				}
			}
		}
	}

	// Creates an entity, simply makes a new id and mask
	EntityID NewEntity()
	{
		m_entities.push_back(ComponentMask());
		return EntityID(m_entities.size() - 1);
	}

	// Makes a new system instance
	template <typename T>
	void RegisterSystem()
	{
		T* pNewSystem = new T();
		m_systems.push_back(pNewSystem);
		pNewSystem->SetSubscriptions();
	}

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template<typename T>
	T* AssignComponent(EntityID id)
	{
		int componentId = GetComponentId<T>();
		if (m_componentPools.size() <= componentId) // Not enough component pool
		{
			m_componentPools.resize(componentId + 1, nullptr);
		}
		if (m_componentPools[componentId] == nullptr) // New component, make a new pool
		{
			m_componentPools[componentId] = new ComponentPool(sizeof(T));
		}

		// Check the mask so you're not overwriting a component
		if (m_entities[id].test(componentId) == false)
		{
			// Looks up the component in the pool, and initializes it with placement new
			// TODO: Fatal error, trying to assign to component that has no pool
			T* pComponent = new (static_cast<T*>(m_componentPools[componentId]->get(id))) T();

			// Set the bit for this component to true
			m_entities[id].set(componentId);
			return pComponent;
		}
		return nullptr;
	}

	// Retrieves a component for a given entity
	// Simply checks the existence using the mask, and then queries the component from the correct pool
	template<typename T>
	T* GetComponent(EntityID id)
	{
		int componentId = GetComponentId<T>();
#ifdef _DEBUG
		// Check to see if the component exists first (only done when not in release builds for extra performance
		assert(m_entities[id].test(componentId)); // TODO: Convert to fatal
#endif // DEBUG
		T* pComponent = static_cast<T*>(m_componentPools[componentId]->get(id));
		return pComponent;
	}

	std::vector<System*> m_systems;

	std::vector<ComponentPool*> m_componentPools;
	std::vector<ComponentMask> m_entities; // TODO: Generational Indicies
};