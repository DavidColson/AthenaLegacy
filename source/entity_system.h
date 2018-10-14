#ifndef ENTITY_SYSTEM_
#define ENTITY_SYSTEM_

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


typedef uint EntityID;
const int MAX_COMPONENTS = 10;
const int MAX_ENTITIES = 100;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

class World;

// *****************************************
// Base class for systems
// *****************************************

class System
{
	friend class World;
public:

	virtual void StartEntity(EntityID id) {};
	virtual void UpdateEntity(EntityID id) = 0;
	virtual void SetSubscriptions() = 0;

protected:
	template <typename T>
	void Subscribe()
	{
		m_componentSubscription.set(gGameWorld.GetComponentTypeId<T>());
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
		data = new char[elementSize * MAX_ENTITIES];
		size = MAX_ENTITIES;
	}

	inline void* get(size_t index)
	{
		return data + index * elementSize;
	}

	char* data;
	size_t elementSize;
	size_t size = 0;
};

// *****************************************
// The game world, holds enties and systems
// Updates systems and manages creation and
// deletion of entities and components
// *****************************************

class World
{
public:
	// TODO Destructor, delete systems and components

	// Goes through each system looping entities and calling the startup function
	// for entities matching the subscription. Called once on scene laod
	void StartSystems()
	{
		for (System* sys : m_systems)
		{
			for (EntityID i = 0; i < m_entities.size(); i++)
			{
				ComponentMask mask = m_entities[i];
				if (sys->m_componentSubscription == (sys->m_componentSubscription & mask))
				{
					sys->StartEntity(i);
				}
			}
		}
	}

	// Goes through each system one by one, looping through all entities and updating the
	// system with entities that match the subscription
	void UpdateSystems()
	{
		for (System* sys : m_systems)
		{
			for (EntityID i = 0; i < m_entities.size(); i++)
			{
				ComponentMask mask = m_entities[i];
				if (sys->m_componentSubscription == (sys->m_componentSubscription & mask))
				{
					sys->UpdateEntity(i);
				}
			}
		}
	}

	// Creates an entity, simply makes a new id and mask
	EntityID NewEntity()
	{
		m_entities.push_back(ComponentMask());
		return uint(m_entities.size() - 1);
	}

	// Makes a new system instance
	template <typename T>
	void RegisterSystem()
	{
		T* newSystem = new T();
		m_systems.push_back(newSystem);
		newSystem->SetSubscriptions();
	}

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template<typename T>
	T* AssignComponent(EntityID id)
	{
		int componentTypeId = GetComponentTypeId<T>();
		if (m_component_pools.size() <= componentTypeId) // Not enough component pool
		{
			m_component_pools.resize(componentTypeId + 1, nullptr);
		}
		if (m_component_pools[componentTypeId] == nullptr) // New component, make a new pool
		{
			m_component_pools[componentTypeId] = new ComponentPool(sizeof(T));
		}

		// Check the mask so you're not overwriting a component
		if (m_entities[id].test(componentTypeId) == false)
		{
			// Looks up the component in the pool, and initializes it with placement new
			// TODO: Fatal error, trying to assign to component that has no pool
			T* pComponent = new (static_cast<T*>(m_component_pools[componentTypeId]->get(id))) T();

			// Set the bit for this component to true
			m_entities[id].set(componentTypeId);
			return pComponent;
		}
		return nullptr;
	}

	// Retrieves a component for a given entity
	// Simply checks the existence using the mask, and then queries the component from the correct pool
	template<typename T>
	T* GetComponent(EntityID id)
	{
		int componentTypeId = GetComponentTypeId<T>();
#ifdef _DEBUG
		// Check to see if the component exists first (only done when not in release builds for extra performance
		assert(m_entities[id].test(componentTypeId)); // TODO: Convert to fatal
#endif // DEBUG
		T* pComponent = static_cast<T*>(m_component_pools[componentTypeId]->get(id));
		return pComponent;
	}

	// Gives you the id within this world for a given component type
	template <class T>
	int GetComponentTypeId()
	{
		// static variable will be initialized on first function call
		// It will then continue to return the same thing, no matter how many times this is called.
		// Allows us to assign a unique id to each component type, since each component type has it's own instance of this function
		static int component_type_id = component_counter++;
		return component_type_id;
	}

private:
	int component_counter = 0;
	std::vector<System*> m_systems;

	std::vector<ComponentPool*> m_component_pools;
	std::vector<ComponentMask> m_entities; // TODO: Generational Indicies
};

// Global instances of game world
World gGameWorld;

#endif