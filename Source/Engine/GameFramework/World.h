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

	std::vector<BaseComponentPool*> m_componentPools;

	struct EntityDesc
	{
		EntityID m_id;
		ComponentMask m_mask;
	};
	std::vector<EntityDesc> m_entities;
	std::vector<EntityIndex> m_freeEntities;
};

// View into the space for a given set of components
template<typename... ComponentTypes>
struct View
{
	View(Space* pSpace) : m_pSpace(pSpace) {
		if (sizeof...(ComponentTypes) == 0)
		{
			m_all = true;
		}
		else
		{
			int componentIds[] = { 0, GetComponentId<ComponentTypes>() ... };
			for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
				m_componentMask.set(componentIds[i]);
		}
	}

	struct Iterator
	{
		Iterator(Space* pSpace, EntityIndex index, ComponentMask mask, bool all) : m_pSpace(pSpace), m_index(index), m_mask(mask), m_all(all) {}

		EntityID operator*() const { return m_pSpace->m_entities[m_index].m_id; }
		bool operator==(const Iterator& other) const { return m_index == other.m_index; }
		bool operator!=(const Iterator& other) const { return m_index != other.m_index; }

		bool ValidIndex()
		{
			return 
				// It's a valid entity ID
				IsEntityValid(m_pSpace->m_entities[m_index].m_id) && 
				// It has the correct component mask
				(m_all || m_mask == (m_mask & m_pSpace->m_entities[m_index].m_mask));
		}

		Iterator& operator++()
		{
			do
			{
				m_index++;
			} while (m_index < m_pSpace->m_entities.size() && !ValidIndex());
			return *this;
		}

		EntityIndex m_index;
		Space* m_pSpace;
		ComponentMask m_mask;
		bool m_all{ false };
	};

	const Iterator begin() const 
	{
		int firstIndex = 0;
		while (m_componentMask != (m_componentMask & m_pSpace->m_entities[firstIndex].m_mask))
		{
			firstIndex++;
		}
		return Iterator(m_pSpace, firstIndex, m_componentMask, m_all);
	}

	const Iterator end() const
	{
		return Iterator(m_pSpace, EntityIndex(m_pSpace->m_entities.size()), m_componentMask, m_all);
	}

	Space* m_pSpace{ nullptr };
	ComponentMask m_componentMask;
	bool m_all{ false };
};