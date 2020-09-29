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

void ShipControlSystemUpdate(Scene& scene, float deltaTime)
{

}

// Then register them with the system
scene.RegisterSystem(SystemPhase::Update, ShipControlSystem);

// Simulate the whole scene by simply calling SimulateScene

scene.SimulateScene(deltaTime);

// To create entities and assign entities to them do this:

EntityID triangle = scene.NewEntity();
Transform* pTransform = scene.Assign<Transform>(triangle);
Shape* pShape = scene.Assign<Shape>(triangle);

EntityID circle = scene.NewEntity();
scene.Assign<Shape>(circle);

// To get the whole thing running, just call your systems ShipControlSystem(scene);

*/

// #TODO: Move some of this to a cpp file

#include "TypeSystem.h"
#include "ErrorHandling.h"
#include "Log.h"
#include "Vec3.h"
#include "Matrix.h"
#include "Engine.h"

#include <EASTL/bitset.h>
#include <EASTL/vector.h>

struct Scene;

typedef uint32_t EntityIndex;

struct EntityID
{
	inline EntityIndex Index() const
	{
		return value >> 32;
	}
	inline uint32_t Version() const
	{
		return uint32_t(value);
	}
	inline bool IsValid() const
	{
		return (value >> 32) != EntityIndex(-1);
	}
	bool operator==(const EntityID& other) const
	{
		return value == other.value;
	}
	bool operator!=(const EntityID& other) const
	{
		return value != other.value;
	}


	static inline EntityID New(EntityIndex index, uint32_t version)
	{
		return EntityID{ ((uint64_t)index << 32) | ((uint64_t)version) };
	}
	static inline EntityID InvalidID()
	{
		return EntityID{ 18446744069414584320 }; // Corresponds to index -1 and version 0
	}

	uint64_t value;
};

const int MAX_COMPONENTS = 25;
const int MAX_ENTITIES = 128;
typedef eastl::bitset<MAX_COMPONENTS> ComponentMask;
typedef void (*ReactiveSystemFunc)(Scene&, EntityID);
typedef void (*SystemFunc)(Scene&, float);

enum class Reaction
{
	OnAdd,
	OnRemove
};

enum class SystemPhase
{
	PreUpdate,
	Update,
	Render
};

// Built-in Components
// ******************

struct CName
{
	eastl::string name;

	REFLECT()
};

struct CVisibility
{
	bool visible{ true };

	REFLECT()
};

struct CTransform
{
	Vec3f localPos{ Vec3f(0.0f) };
	Vec3f localSca{ Vec3f(1.0f) };
	Vec3f localRot{ Vec3f(0.0f) };

	Matrixf globalTransform;

	REFLECT()
};

struct CParent
{
	int nChildren{ 0 };
	EntityID firstChild{ EntityID::InvalidID() };

	REFLECT()
};

struct CChild
{
	EntityID parent{ EntityID::InvalidID() };
	
	// doubly linked list of children, 
	// this method is used because it requires no dynamic allocation
	EntityID prev{ EntityID::InvalidID() };
	EntityID next{ EntityID::InvalidID() };

	REFLECT();
};

// Built In Systems
// ****************

void TransformHeirarchy(Scene& scene, float deltaTime);

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
	ASSERT(s_componentId < MAX_COMPONENTS, "Too many component types, above supported amount");
	return s_componentId;
}


// ********************************************
// All components are stored in component pools
// Memory is managed manually
// ********************************************

// #TODO: Move this inside the scene struct, no one should need to touch this
struct BaseComponentPool
{
	BaseComponentPool(size_t elementsize);

	~BaseComponentPool();

	inline void* get(size_t index)
	{
		ASSERT(index < MAX_ENTITIES, "Entity overrun, delete some entities");
		return pData + index * elementSize;
	}

	virtual void destroy(size_t index) = 0;

	char* pData{ nullptr };
	size_t elementSize{ 0 };
	TypeData* pTypeData{ nullptr };

	// Systems that have requested to know when this component has been added or removed from an entity
	eastl::vector<ReactiveSystemFunc> onAddedCallbacks;
	eastl::vector<ReactiveSystemFunc> onRemovedCallbacks;
};

template <typename T>
struct ComponentPool : public BaseComponentPool // #TODO: Move to detail namespace
{
	ComponentPool(size_t elementsize) : BaseComponentPool(elementsize) { pTypeData = &TypeDatabase::Get<T>(); }

	// TODO: This could potentially be moved to be inside TypeData and applied to a variant of the component
	virtual void destroy(size_t index) override
	{
		ASSERT(index < MAX_ENTITIES, "Trying to delete an entity with an ID greater than max allowed entities");
		static_cast<T*>(get(index))->~T();
	}
};

// A wrapper for storing the templated functions for dealing with a specific function. Used when handling components without the actual type involved
struct ComponentHandler
{
	virtual void* Assign(Scene& scene, EntityID entity) = 0;
	virtual void Remove(Scene& scene, EntityID entity) = 0;
};

template<typename T>
struct ComponentHandler_Internal : public ComponentHandler
{
	virtual void* Assign(Scene& scene, EntityID entity) override
	{
		return scene.Assign<T>(entity);
	}

	virtual void Remove(Scene& scene, EntityID entity) override
	{
		scene.Remove<T>(entity);
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
	struct EntityDesc
	{
		EntityID id;
		ComponentMask mask;
	};

	Scene();

	~Scene();

	// Creates an entity, simply makes a new id and mask
	EntityID NewEntity(const char* name);

	void DestroyEntity(EntityID id);

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template<typename T>
	T* Assign(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = GetId<T>();
		if (componentPools.size() <= componentId) // Not enough component pool
		{
			componentPools.resize(componentId + 1, nullptr);
		}
		if (componentPools[componentId] == nullptr) // New component, make a new pool
		{
			componentPools[componentId] = new ComponentPool<T>(sizeof(T));
		}

		// Check the mask so you're not overwriting a component
		ASSERT(Has<T>(id) == false, "You're trying to assign a component to an entity that already has this component");
		
		// Looks up the component in the pool, and initializes it with placement new
		T* pComponent = new (componentPools[componentId]->get(id.Index())) T();

		// Set the bit for this component to true
		entities[id.Index()].mask.set(componentId);

		// Send OnAdd events
		for (ReactiveSystemFunc func : componentPools[componentId]->onAddedCallbacks)
		{
			func(*this, id);
		}
		
		return pComponent;
	}

	template<typename T>
	void Remove(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return;

		int componentId = GetId<T>();
		ASSERT(Has<T>(id), "The component you're trying to access is not assigned to this entity");
		
		// Send OnRemoved events
		for (ReactiveSystemFunc func : componentPools[componentId]->onRemovedCallbacks)
		{
			func(*this, id);
		}
 		
		 // Turn off the component bit
		entities[id.Index()].mask.reset(componentId);
		// Destroy the component
		componentPools[componentId]->destroy(id.Index());
	}

	// Retrieves a component for a given entity
	// Simply checks the existence using the mask, and then queries the component from the correct pool
	template<typename T>
	T* Get(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = GetId<T>();
		ASSERT(Has<T>(id), "The component you're trying to access is not assigned to this entity");
		T* pComponent = static_cast<T*>(componentPools[componentId]->get(id.Index()));
		return pComponent;
	}

	// Checks if an entity with given Id has a component of type T assigned to it
	template<typename T>
	bool Has(EntityID id)
	{
		if (!id.IsValid() || entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return false;

		int componentId = GetId<T>();
		return entities[id.Index()].mask.test(componentId);
	}

	eastl::string GetEntityName(EntityID entity);

	template<typename T>
	void RegisterReactiveSystem(Reaction reaction, ReactiveSystemFunc func)
	{
		int componentId = GetId<T>();
		if (componentPools.size() <= componentId) // Not enough component pool
		{
			componentPools.resize(componentId + 1, nullptr);
		}
		if (componentPools[componentId] == nullptr) // New component, make a new pool
		{
			componentPools[componentId] = new ComponentPool<T>(sizeof(T));
		}

		switch (reaction)
		{
		case Reaction::OnAdd:
			componentPools[componentId]->onAddedCallbacks.push_back(func);
			break;
		case Reaction::OnRemove:
			componentPools[componentId]->onRemovedCallbacks.push_back(func);
			break;
		default:
			break;
		}
	}

	void SetParent(EntityID child, EntityID parent);

	void UnsetParent(EntityID child, EntityID parent);

	void RegisterSystem(SystemPhase phase, SystemFunc func);

	void SimulateScene(float deltaTime);

	void RenderScene(float deltaTime);

	eastl::vector<BaseComponentPool*> componentPools;

	int nActiveEntities{0};
	eastl::vector<EntityDesc> entities;
	eastl::vector<EntityIndex> freeEntities; // We just store the indices here, not a full EntityID

	eastl::vector<SystemFunc> preUpdateSystems;
	eastl::vector<SystemFunc> updateSystems;
	eastl::vector<SystemFunc> renderSystems;
};

// View into the Scene for a given set of components
template<typename... ComponentTypes>
struct SceneView
{
	template <size_t comps = sizeof...(ComponentTypes), eastl::enable_if_t<comps == 0, int> = 0>
	SceneView(Scene& scene) : pScene(&scene) {
		all = true;
	}

	template <size_t comps = sizeof...(ComponentTypes), eastl::enable_if_t<comps != 0, int> = 0>
	SceneView(Scene& scene) : pScene(&scene) {
		int componentIds[] = { 0, GetId<ComponentTypes>() ... };
		for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
			componentMask.set(componentIds[i]);
	}

	struct Iterator
	{
		Iterator(Scene* pScene, EntityIndex index, ComponentMask mask, bool all) : pScene(pScene), index(index), mask(mask), all(all) {}

		EntityID operator*() const { return pScene->entities[index].id; }
		bool operator==(const Iterator& other) const 
		{
			return index == other.index || index == pScene->entities.size(); 
		}
		bool operator!=(const Iterator& other) const 
		{
			return index != other.index && index != pScene->entities.size();
		}

		bool ValidIndex()
		{
			return 
				// It's a valid entity ID
				pScene->entities[index].id.IsValid() && 
				// It has the correct component mask
				(all || mask == (mask & pScene->entities[index].mask));
		}

		Iterator& operator++()
		{
			do
			{
				index++;
			} while (index < pScene->entities.size() && !ValidIndex());
			return *this;
		}

		EntityIndex index;
		Scene* pScene;
		ComponentMask mask;
		bool all{ false };
	};

	const Iterator begin() const 
	{
		int firstIndex = 0;
		while (firstIndex < pScene->entities.size() && // Checking we're not overflowing
			(componentMask != (componentMask & pScene->entities[firstIndex].mask) // Does this index have the right components?
			|| !pScene->entities[firstIndex].id.IsValid())) // Does this index have a valid entity?
		{
			firstIndex++;
		}
		return Iterator(pScene, firstIndex, componentMask, all);
	}

	const Iterator end() const
	{
		return Iterator(pScene, EntityIndex(pScene->entities.size()), componentMask, all);
	}

	Scene* pScene{ nullptr };
	ComponentMask componentMask;
	bool all{ false };
};