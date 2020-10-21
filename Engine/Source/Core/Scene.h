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

const int MAX_COMPONENTS = 50;
const int MAX_ENTITIES = 1024;
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
	bool visible{true};

	REFLECT()
};

struct CTransform
{
	Vec3f localPos{Vec3f(0.0f)};
	Vec3f localSca{Vec3f(1.0f)};
	Vec3f localRot{Vec3f(0.0f)};

	Matrixf globalTransform;

	REFLECT()
};

struct CParent
{
	int nChildren{0};
	EntityID firstChild{EntityID::InvalidID()};

	REFLECT()
};

struct CChild
{
	EntityID parent{EntityID::InvalidID()};

	// doubly linked list of children,
	// this method is used because it requires no dynamic allocation
	EntityID prev{EntityID::InvalidID()};
	EntityID next{EntityID::InvalidID()};

	REFLECT();
};

// Built In Systems
// ****************

void TransformHeirarchy(Scene &scene, float deltaTime);



// ********************************************
// All components are stored in component pools
// Memory is managed manually
// ********************************************

// #TODO: Move this inside the scene struct, no one should need to touch this
struct ComponentPool
{
	ComponentPool(size_t elementsize, TypeData& typeData, void (*_pDestructor)(void *, TypeData*));

	~ComponentPool();

	inline void *GetRaw(size_t index)
	{
		ASSERT(index < MAX_ENTITIES, "Entity overrun, delete some entities");
		return pData + index * elementSize;
	}

	inline void Erase(size_t index)
	{
		pDestructor(GetRaw(index), pTypeData);
	}

	void (*pDestructor)(void *, TypeData*);

	char *pData{nullptr};
	size_t elementSize{0};
	TypeData *pTypeData{nullptr};

	// Systems that have requested to know when this component has been added or removed from an entity
	eastl::vector<ReactiveSystemFunc> onAddedCallbacks;
	eastl::vector<ReactiveSystemFunc> onRemovedCallbacks;
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
	EntityID NewEntity(const char *name);

	void DestroyEntity(EntityID id);

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template <typename T>
	T *Assign(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		ComponentPool *pPool = GetOrCreateComponentPool<T>();
		ASSERT(Has<T>(id) == false, "You're trying to assign a component to an entity that already has this component");

		// Looks up the component in the pool, and initializes it with placement new
		T *pComponent = new (pPool->GetRaw(id.Index())) T();

		// Set the bit for this component to true
		int componentId = Type::Index<T>();
		entities[id.Index()].mask.set(componentId);

		// Send OnAdd events
		for (ReactiveSystemFunc func : pPool->onAddedCallbacks)
		{
			func(*this, id);
		}

		return pComponent;
	}

	void Assign(EntityID id, TypeData& componentType)
	{
		if (entities[id.Index()].id != id)
			return;

		ComponentPool *pPool = GetOrCreateComponentPool(componentType);
		ASSERT(Has(id, componentType) == false, "You're trying to assign a component to an entity that already has this component");

		componentType.pConstructor->PlacementNew(pPool->GetRaw(id.Index()));

		entities[id.Index()].mask.set(componentType.id);

		for (ReactiveSystemFunc func : pPool->onAddedCallbacks)
		{
			func(*this, id);
		}
	}

	template <typename T>
	void Remove(EntityID id)
	{
		if (entities[id.Index()].id != id) 
			return;

		int componentId = Type::Index<T>();
		ASSERT(Has<T>(id), "The component you're trying to access is not assigned to this entity");

		for (ReactiveSystemFunc func : componentPools[componentId]->onRemovedCallbacks)
		{
			func(*this, id);
		}
		entities[id.Index()].mask.reset(componentId);
		componentPools[componentId]->Erase(id.Index());
	}

	void Remove(EntityID id, TypeData& componentType)
	{
		if (entities[id.Index()].id != id)
			return;

		ASSERT(Has(id, componentType), "The component you're trying to access is not assigned to this entity");

		int componentId = componentType.id;
		for (ReactiveSystemFunc func : componentPools[componentId]->onRemovedCallbacks)
		{
			func(*this, id);
		}
		entities[id.Index()].mask.reset(componentId);
		componentPools[componentId]->Erase(id.Index());
	}

	// Retrieves a component for a given entity
	// Simply checks the existence using the mask, and then queries the component from the correct pool
	template <typename T>
	T *Get(EntityID id) // #TODO: Move implementation to lower down in the file
	{
		if (entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return nullptr;

		int componentId = Type::Index<T>();

		ASSERT(Has<T>(id), "The component you're trying to access is not assigned to this entity");
		T *pComponent = static_cast<T *>(componentPools[componentId]->GetRaw(id.Index()));
		return pComponent;
	}

	Variant Get(EntityID id, TypeData& componentType)
	{
		if (entities[id.Index()].id != id)
			return nullptr;

		ASSERT(Has(id, componentType), "The component you're trying to access is not assigned to this entity");

		void* pData = componentPools[componentType.id]->GetRaw(id.Index());
		return componentType.pConstructor->CopyToVariant(pData);
	}

	void Set(EntityID id, Variant componentToSet)
	{
		if (entities[id.Index()].id != id)
			return;
		
		int componentId = componentToSet.GetType().id;

		ASSERT(Has(id, componentToSet.GetType()), "The component you're trying to set data on is not assigned to this entity");

		void* pData = componentPools[componentId]->GetRaw(id.Index());
		memcpy(pData, componentToSet.pData, componentToSet.GetType().size);
	}

	// Checks if an entity with given Id has a component of type T assigned to it
	template <typename T>
	bool Has(EntityID id)
	{
		if (!id.IsValid() || entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return false;

		return entities[id.Index()].mask.test(Type::Index<T>());
	}

	bool Has(EntityID id, TypeData &type)
	{
		if (!id.IsValid() || entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
			return false;

		return entities[id.Index()].mask.test(type.id);
	}

	eastl::string GetEntityName(EntityID entity);

	template <typename T>
	void RegisterReactiveSystem(Reaction reaction, ReactiveSystemFunc func)
	{
		ComponentPool *pPool = GetOrCreateComponentPool<T>();

		switch (reaction)
		{
		case Reaction::OnAdd:
			pPool->onAddedCallbacks.push_back(func);
			break;
		case Reaction::OnRemove:
			pPool->onRemovedCallbacks.push_back(func);
			break;
		default:
			break;
		}
	}

	void RegisterReactiveSystem(Reaction reaction, ReactiveSystemFunc func, TypeData& type)
	{
		ComponentPool *pPool = GetOrCreateComponentPool(type);

		switch (reaction)
		{
		case Reaction::OnAdd:
			pPool->onAddedCallbacks.push_back(func);
			break;
		case Reaction::OnRemove:
			pPool->onRemovedCallbacks.push_back(func);
			break;
		default:
			break;
		}
	}

	template <typename T>
	ComponentPool* GetOrCreateComponentPool()
	{
		uint32_t componentId = Type::Index<T>();
		if (componentPools.size() <= componentId) // Not enough component pool
			componentPools.resize(componentId + 1, nullptr);

		if (componentPools[componentId] == nullptr) // New component, make a new pool
		{
			componentPools[componentId] = new ComponentPool(sizeof(T), TypeDatabase::Get<T>(), [](void *pComponent, TypeData* pTypeData) {
				static_cast<T *>(pComponent)->~T();
			});
		}
		return componentPools[componentId];
	}

	ComponentPool* GetOrCreateComponentPool(TypeData& type)
	{
		uint32_t componentId = type.id;
		if (componentPools.size() <= componentId) // Not enough component pool
			componentPools.resize(componentId + 1, nullptr);

		if (componentPools[componentId] == nullptr) // New component, make a new pool
		{	
			componentPools[componentId] = new ComponentPool(type.size, type, [](void *pComponent, TypeData* pTypeData) {
				pTypeData->pConstructor->Destruct(pComponent);
			});
		}
		return componentPools[componentId];
	}

	void SetParent(EntityID child, EntityID parent);

	void UnsetParent(EntityID child, EntityID parent);

	void RegisterSystem(SystemPhase phase, SystemFunc func);

	void SimulateScene(float deltaTime);

	void RenderScene(float deltaTime);

	eastl::vector<ComponentPool *> componentPools;

	int nActiveEntities{0};
	eastl::vector<EntityDesc> entities;
	eastl::vector<EntityIndex> freeEntities; // We just store the indices here, not a full EntityID

	eastl::vector<SystemFunc> preUpdateSystems;
	eastl::vector<SystemFunc> updateSystems;
	eastl::vector<SystemFunc> renderSystems;
};

// View into the Scene for a given set of components
template <typename... ComponentTypes>
struct SceneView
{
	template <size_t comps = sizeof...(ComponentTypes), eastl::enable_if_t<comps == 0, int> = 0>
	SceneView(Scene &scene) : pScene(&scene)
	{
		all = true;
	}

	template <size_t comps = sizeof...(ComponentTypes), eastl::enable_if_t<comps != 0, int> = 0>
	SceneView(Scene &scene) : pScene(&scene)
	{
		uint32_t componentIndexes[] = {0, Type::Index<ComponentTypes>()...};
		for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
			componentMask.set(componentIndexes[i]);
	}

	struct Iterator
	{
		Iterator(Scene *pScene, EntityIndex index, ComponentMask mask, bool all) : pScene(pScene), index(index), mask(mask), all(all) {}

		EntityID operator*() const { return pScene->entities[index].id; }
		bool operator==(const Iterator &other) const
		{
			return index == other.index || index == pScene->entities.size();
		}
		bool operator!=(const Iterator &other) const
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

		Iterator &operator++()
		{
			do
			{
				index++;
			} while (index < pScene->entities.size() && !ValidIndex());
			return *this;
		}

		EntityIndex index;
		Scene *pScene;
		ComponentMask mask;
		bool all{false};
	};

	const Iterator begin() const
	{
		int firstIndex = 0;
		while (firstIndex < pScene->entities.size() &&								 // Checking we're not overflowing
			   (componentMask != (componentMask & pScene->entities[firstIndex].mask) // Does this index have the right components?
				|| !pScene->entities[firstIndex].id.IsValid()))						 // Does this index have a valid entity?
		{
			firstIndex++;
		}
		return Iterator(pScene, firstIndex, componentMask, all);
	}

	const Iterator end() const
	{
		return Iterator(pScene, EntityIndex(pScene->entities.size()), componentMask, all);
	}

	Scene *pScene{nullptr};
	ComponentMask componentMask;
	bool all{false};
};