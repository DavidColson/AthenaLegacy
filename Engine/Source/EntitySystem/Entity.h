#pragma once

#include "EASTL/vector.h"
#include "EASTL/string.h"
#include "UUID.h"

class IEntitySystem;
struct UpdateContext;

struct IComponent
{
    IComponent() : id(Uuid::New()) {}

    Uuid id;
    REFLECT_DERIVED()
};

class Entity
{
public:
	Entity() : id(Uuid::New()) {}

    Uuid GetId() { return id; }

    // This function will loop through components and register them with the systems
	[[nodiscard]] eastl::vector<IComponent*> Activate();

	// Loop through components and unregister them with systems
	[[nodiscard]] eastl::vector<IComponent*> Deactivate();

	// Loops through systems updating them
	void Update(UpdateContext& ctx);
    
    template<typename Type>
    Type* AddNewComponent()
    {
        Type* pComponent = new Type();
        components.push_back(static_cast<IComponent*>(pComponent));
        return pComponent;
    }

	template<typename Type>
    Type* AddNewSystem()
    {
        Type* pSystem = new Type();
        systems.push_back(static_cast<IEntitySystem*>(pSystem));
        return pSystem;
    }

	eastl::string name;

private:
    Uuid id;

	eastl::vector<IComponent*> components;
	eastl::vector<IEntitySystem*> systems;
};