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

typedef unsigned int EntityIndex;
typedef unsigned int EntityVersion;
typedef unsigned long long EntityID;
const int MAX_COMPONENTS = 10;
const int MAX_ENTITIES = 64;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

struct Space;

// Move into the detail namespace
inline EntityID CreateEntityId(EntityIndex index, EntityVersion version)
{
	return ((EntityID)index << 32) | ((EntityID)version);
}
inline EntityIndex GetEntityIndex(EntityID id)
{
	return id >> 32;
}
inline EntityVersion GetEntityVersion(EntityID id)
{
	return (EntityVersion)id;
}
inline bool IsEntityValid(EntityID id)
{
	return (id >> 32) != EntityIndex(-1);
}



// Gives you the id within this world for a given component type
extern int s_componentCounter; // TODO: Move this to a detail namespace
template <class T>
int GetComponentId() // Move this whole function to the detail namespace
{
	// static variable will be initialized on first function call
	// It will then continue to return the same thing, no matter how many times this is called.
	// Allows us to assign a unique id to each component type, since each component type has it's own instance of this function
	static int s_componentId = s_componentCounter++;
	return s_componentId;
}



// Used to relate components to the Type objects in the reflection database
struct ComponentIdToTypeIdMap // TODO: Move to detail namespace
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
// ********************************************

struct BaseComponentPool // TODO: Move to detail namespace
{
	BaseComponentPool(size_t elementsize)
	{
		elementSize = elementsize;
		pData = new char[elementSize * MAX_ENTITIES];
		size = MAX_ENTITIES;
	}

	inline void* get(size_t index)
	{
		ASSERT(index < size, "Entity overrun, delete some entities");
		return pData + index * elementSize;
	}

	virtual void destroy(size_t index) = 0;

	char* pData;
	size_t elementSize;
	size_t size = 0;
};

template <typename T>
struct ComponentPool : public BaseComponentPool // TODO: Move to detail namespace
{
	ComponentPool(size_t elementsize) : BaseComponentPool(elementsize) {}

	virtual void destroy(size_t index) override
	{
		ASSERT(index < size, "Trying to delete an entity with an ID greater than max allowed entities");
		static_cast<T*>(get(index))->~T();
	}
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
	void StartSystems() // TODO: Move implementation to cpp
	{
		for (System* pSys : m_systems)
		{
			for (EntityIndex i = 0; i < m_entities.size(); i++)
			{
				ComponentMask mask = m_entities[i].m_mask;
				if (IsEntityValid(m_entities[i].m_id) && pSys->m_componentSubscription == (pSys->m_componentSubscription & mask))
				{
					pSys->StartEntity(m_entities[i].m_id, this);
				}
			}
		}
	}

	// Goes through each system one by one, looping through all entities and updating the
	// system with entities that match the subscription
	void UpdateSystems(float deltaTime)// TODO: Move implementation to cpp
	{
		for (System* sys : m_systems)
		{
			for (EntityIndex i = 0; i < m_entities.size(); i++)
			{
				ComponentMask mask = m_entities[i].m_mask;
				if (IsEntityValid(m_entities[i].m_id) && sys->m_componentSubscription == (sys->m_componentSubscription & mask) && !sys->m_componentSubscription.none())
				{
					sys->UpdateEntity(m_entities[i].m_id, this, deltaTime);
				}
			}
		}
	}

	// Creates an entity, simply makes a new id and mask
	EntityID NewEntity()// TODO: Move implementation to cpp
	{
 		if (!m_freeEntities.empty())
		{
			EntityIndex newIndex = m_freeEntities.back();
			m_freeEntities.pop_back();
			m_entities[newIndex].m_id = CreateEntityId(newIndex, GetEntityVersion(m_entities[newIndex].m_id));
			return m_entities[newIndex].m_id;
		}
		m_entities.push_back({ CreateEntityId(EntityIndex(m_entities.size()), 0), ComponentMask() });
		return m_entities.back().m_id;
	}

	void DestroyEntity(EntityID id)
	{
		for (int i = 0; i < MAX_COMPONENTS; i++)
		{
			// For each component ID, check the bitmask, if no, continue, if yes, destroy the component
			std::bitset<MAX_COMPONENTS> mask;
			mask.set(i, true);
			if (mask == (m_entities[GetEntityIndex(id)].m_mask & mask))
			{
				m_componentPools[i]->destroy(GetEntityIndex(id));
			}
		}
		m_entities[GetEntityIndex(id)].m_id = CreateEntityId(EntityIndex(-1), GetEntityVersion(id) + 1); // set to invalid
		m_entities[GetEntityIndex(id)].m_mask.reset(); // clear components
		m_freeEntities.push_back(GetEntityIndex(id));
	}

	// Makes a new system instance
	template <typename T>
	void RegisterSystem() // TODO: Move implementation to lower down in the file
	{
		T* pNewSystem = new T();
		m_systems.push_back(pNewSystem);
		pNewSystem->SetSubscriptions();
	}

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template<typename T>
	T* AssignComponent(EntityID id) // TODO: Move implementation to lower down in the file
	{
		if (m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = GetComponentId<T>();
		if (m_componentPools.size() <= componentId) // Not enough component pool
		{
			m_componentPools.resize(componentId + 1, nullptr);
		}
		if (m_componentPools[componentId] == nullptr) // New component, make a new pool
		{
			m_componentPools[componentId] = new ComponentPool<T>(sizeof(T));
		}

		// Check the mask so you're not overwriting a component
		ASSERT(HasComponent<T>(id) == false, "You're trying to assign a component to an entity that already has this component");
		
		// Looks up the component in the pool, and initializes it with placement new
		T* pComponent = new (static_cast<T*>(m_componentPools[componentId]->get(GetEntityIndex(id)))) T();

		// Set the bit for this component to true
		m_entities[GetEntityIndex(id)].m_mask.set(componentId);
		return pComponent;
	}

	// Retrieves a component for a given entity
	// Simply checks the existence using the mask, and then queries the component from the correct pool
	template<typename T>
	T* GetComponent(EntityID id) // TODO: Move implementation to lower down in the file
	{
		if (m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = GetComponentId<T>();
		ASSERT(HasComponent<T>(id), "The component you're trying to access is not assigned to this entity");
		T* pComponent = static_cast<T*>(m_componentPools[componentId]->get(GetEntityIndex(id)));
		return pComponent;
	}

	// Checks if an entity with given Id has a component of type T assigned to it
	template<typename T>
	bool HasComponent(EntityID id)
	{
		if (m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return false;

		int componentId = GetComponentId<T>();
		return m_entities[GetEntityIndex(id)].m_mask.test(componentId);
	}

	std::vector<System*> m_systems;

	std::vector<BaseComponentPool*> m_componentPools;

	struct EntityDesc
	{
		EntityID m_id;
		ComponentMask m_mask;
	};
	std::vector<EntityDesc> m_entities;
	std::vector<EntityIndex> m_freeEntities;
};