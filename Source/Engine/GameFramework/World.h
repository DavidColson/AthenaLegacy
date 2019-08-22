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

void ShipControlSystemUpdate(Scene* pScene, float deltaTime)
{

}

// Update the system by simply calling it with the current scene

ShipControlSystem(pScene, deltaTime);

// To create entities and assign entities to them do this:

EntityID triangle = gGameWorld.NewEntity();
Transform* pTransform = gGameWorld.Assign<Transform>(triangle);
Shape* pShape = gGameWorld.Assign<Shape>(triangle);

EntityID circle = gGameWorld.NewEntity();
gGameWorld.Assign<Shape>(circle);

// To get the whole thing running, just call gGameWorld.UpdateSystems();

*/

// #TODO: Move some of this to a cpp file

#include "TypeDB.h"

#include <bitset>
#include <vector>

typedef unsigned int EntityIndex;
typedef unsigned int EntityVersion;
typedef unsigned long long EntityID;
const int MAX_COMPONENTS = 10;
const int MAX_ENTITIES = 64;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

struct Scene;

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

#define INVALID_ENTITY CreateEntityId(EntityIndex(-1), 0)




// Gives you the id within this world for a given component type
extern int s_componentCounter; // #TODO: Move this to a detail namespace
template <class T>
int GetId() // Move this whole function to the detail namespace
{
	// static variable will be initialized on first function call
	// It will then continue to return the same thing, no matter how many times this is called.
	// Allows us to assign a unique id to each component type, since each component type has it's own instance of this function
	// NOTE THIS IS NOT THREADSAFE PROBABLY DO SOMETHING ABOUT THAT
	static int s_componentId = s_componentCounter++;
	return s_componentId;
}


// ********************************************
// All components are stored in component pools
// Memory is managed manually
// ********************************************

struct BaseComponentPool // #TODO: Move to detail namespace
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
	TypeId componentTypeId = 0;
};

template <typename T>
struct ComponentPool : public BaseComponentPool // #TODO: Move to detail namespace
{
	ComponentPool(size_t elementsize) : BaseComponentPool(elementsize) { componentTypeId = TypeDB::TypeIdGenerator<T>::Id(); }

	virtual void destroy(size_t index) override
	{
		ASSERT(index < size, "Trying to delete an entity with an ID greater than max allowed entities");
		static_cast<T*>(get(index))->~T();
	}
};

// *****************************************
// A game scene, holds enties and systems
// Updates systems and manages creation and
// deletion of entities and components
// You should be able to maintain multiple scenes at once
// *****************************************

struct Scene
{
	// #TODO Destructor, delete systems and components

	// Creates an entity, simply makes a new id and mask
	EntityID NewEntity(const char* name)// #TODO: Move implementation to cpp
	{
 		if (!m_freeEntities.empty())
		{
			EntityIndex newIndex = m_freeEntities.back();
			m_freeEntities.pop_back();
			m_entities[newIndex].m_id = CreateEntityId(newIndex, GetEntityVersion(m_entities[newIndex].m_id));
			m_entities[newIndex].m_name = name;
			return m_entities[newIndex].m_id;
		}
		m_entities.push_back({ CreateEntityId(EntityIndex(m_entities.size()), 0), name, ComponentMask() });
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

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template<typename T>
	T* Assign(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = GetId<T>();
		if (m_componentPools.size() <= componentId) // Not enough component pool
		{
			m_componentPools.resize(componentId + 1, nullptr);
		}
		if (m_componentPools[componentId] == nullptr) // New component, make a new pool
		{
			m_componentPools[componentId] = new ComponentPool<T>(sizeof(T));
		}

		// Check the mask so you're not overwriting a component
		ASSERT(Has<T>(id) == false, "You're trying to assign a component to an entity that already has this component");
		
		// Looks up the component in the pool, and initializes it with placement new
		T* pComponent = new (static_cast<T*>(m_componentPools[componentId]->get(GetEntityIndex(id)))) T();

		// Set the bit for this component to true
		m_entities[GetEntityIndex(id)].m_mask.set(componentId);
		return pComponent;
	}

	template<typename T>
	void Remove(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return;

		int componentId = GetId<T>();
		ASSERT(Has<T>(id), "The component you're trying to access is not assigned to this entity");
		m_entities[GetEntityIndex(id)].m_mask.reset(componentId); // Turn off the component bit
	}


	// Retrieves a component for a given entity
	// Simply checks the existence using the mask, and then queries the component from the correct pool
	template<typename T>
	T* Get(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = GetId<T>();
		ASSERT(Has<T>(id), "The component you're trying to access is not assigned to this entity");
		T* pComponent = static_cast<T*>(m_componentPools[componentId]->get(GetEntityIndex(id)));
		return pComponent;
	}

	// Checks if an entity with given Id has a component of type T assigned to it
	template<typename T>
	bool Has(EntityID id)
	{
		if (!IsEntityValid(id) || m_entities[GetEntityIndex(id)].m_id != id) // ensures you're not accessing an entity that has been deleted
			return false;

		int componentId = GetId<T>();
		return m_entities[GetEntityIndex(id)].m_mask.test(componentId);
	}

	const char* GetEntityName(EntityID entity) const
	{
		return m_entities[GetEntityIndex(entity)].m_name;
	}

	std::vector<BaseComponentPool*> m_componentPools;

	struct EntityDesc
	{
		EntityID m_id;
		const char* m_name; // #RefactorNote: Move this to a component, store no data in here that is not necessary.
		ComponentMask m_mask;
	};
	std::vector<EntityDesc> m_entities;
	std::vector<EntityIndex> m_freeEntities;
};

// View into the Scene for a given set of components
template<typename... ComponentTypes>
struct SceneView
{
	SceneView(Scene* pScene) : m_pScene(pScene) {
		if (sizeof...(ComponentTypes) == 0)
		{
			m_all = true;
		}
		else
		{
			int componentIds[] = { 0, GetId<ComponentTypes>() ... };
			for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
				m_componentMask.set(componentIds[i]);
		}
	}

	struct Iterator
	{
		Iterator(Scene* pScene, EntityIndex index, ComponentMask mask, bool all) : m_pScene(pScene), m_index(index), m_mask(mask), m_all(all) {}

		EntityID operator*() const { return m_pScene->m_entities[m_index].m_id; }
		bool operator==(const Iterator& other) const 
		{
			return m_index == other.m_index || m_index == m_pScene->m_entities.size(); 
		}
		bool operator!=(const Iterator& other) const 
		{
			return m_index != other.m_index && m_index != m_pScene->m_entities.size();
		}

		bool ValidIndex()
		{
			return 
				// It's a valid entity ID
				IsEntityValid(m_pScene->m_entities[m_index].m_id) && 
				// It has the correct component mask
				(m_all || m_mask == (m_mask & m_pScene->m_entities[m_index].m_mask));
		}

		Iterator& operator++()
		{
			do
			{
				m_index++;
			} while (m_index < m_pScene->m_entities.size() && !ValidIndex());
			return *this;
		}

		EntityIndex m_index;
		Scene* m_pScene;
		ComponentMask m_mask;
		bool m_all{ false };
	};

	const Iterator begin() const 
	{
		int firstIndex = 0;
		while (firstIndex < m_pScene->m_entities.size() && // Checking we're not overflowing
			(m_componentMask != (m_componentMask & m_pScene->m_entities[firstIndex].m_mask) // Does this index have the right components?
			|| !IsEntityValid(m_pScene->m_entities[firstIndex].m_id))) // Does this index have a valid entity?
		{
			firstIndex++;
		}
		return Iterator(m_pScene, firstIndex, m_componentMask, m_all);
	}

	const Iterator end() const
	{
		return Iterator(m_pScene, EntityIndex(m_pScene->m_entities.size()), m_componentMask, m_all);
	}

	Scene* m_pScene{ nullptr };
	ComponentMask m_componentMask;
	bool m_all{ false };
};