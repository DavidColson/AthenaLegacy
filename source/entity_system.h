

typedef uint EntityID;
const int MAX_COMPONENTS = 10;
const int MAX_ENTITIES = 100;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

// *****************************************
// Base class for systems
// *****************************************

class System
{
public:
	ComponentMask m_componentSubscription;

	virtual void UpdateEntity(EntityID id) = 0;
	virtual void Subscribe() = 0;
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
		newSystem->Subscribe();
	}

	// Assigns a component to an entity, optionally making a new memory pool for a new component
	// Will not make components on entities that already have that component
	template<typename T>
	T* AssignComponent(EntityID id)
	{
		int componentTypeId = GetComponentTypeId<T>();
		if (componentTypeId != m_componentPools.size() - 1) // this is a never before seen component
		{
			m_componentPools.push_back(new ComponentPool(sizeof(T)));
		}

		// Check the mask so you're not overwriting a component
		if (m_entities[id].test(componentTypeId) == false)
		{
			// Looks up the component in the pool, and initializes it with placement new
			T* pComponent = new (static_cast<T*>(m_componentPools[componentTypeId]->get(id))) T();

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
		// Check to see if the component exists first
		if (m_entities[id].test(componentTypeId))
		{
			T* pComponent = static_cast<T*>(m_componentPools[componentTypeId]->get(id));
			return pComponent;
		}
		return nullptr;
	}

	// Gives you the id within this world for a given component type
	int componentCounter = 0;
	template <class T>
	int GetComponentTypeId()
	{
		// static variable will be initialized on first function call
		// It will then continue to return the same thing, no matter how many times this is called.
		// Allows us to assign a unique id to each component type, since each component type has it's own instance of this function
		static int componentTypeId = componentCounter++;
		return componentTypeId;
	}

	std::vector<System*> m_systems;

	std::vector<ComponentPool*> m_componentPools;
	std::vector<ComponentMask> m_entities;
};

// Global instances of game world
World gGameWorld;