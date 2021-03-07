#pragma once

#include "EASTL/vector.h"
#include "EASTL/string.h"

class ISystem;

struct IComponent
{
    REFLECT_DERIVED()
};

class Entity
{
public:
	// This function will loop through components and register them with the systems
	[[nodiscard]] eastl::vector<IComponent*> Activate();

	// Loop through components and unregister them with systems
	[[nodiscard]] eastl::vector<IComponent*> Deactivate();

	// Loops through systems updating them
	void Update(float deltaTime);
    
    template<typename Type>
    Type* AddNewComponent()
    {
        Type* pComponent = new Type();
        components.push_back(static_cast<IComponent*>(pComponent));
        return pComponent;
    }

	eastl::string name;

private:
	eastl::vector<IComponent*> components;
	eastl::vector<ISystem*> systems;
};