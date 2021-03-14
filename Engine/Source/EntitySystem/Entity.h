#pragma once

#include "EASTL/vector.h"
#include "EASTL/string.h"
#include "UUID.h"
#include "IComponent.h"

class IEntitySystem;
struct UpdateContext;
struct SpatialComponent;

class Entity
{
public:
	Entity() : id(Uuid::New()) {}

    Uuid GetId() const { return id; }

    // This function will loop through components and register them with the systems
	[[nodiscard]] eastl::vector<IComponent*> Activate();

	// Loop through components and unregister them with systems
	[[nodiscard]] eastl::vector<IComponent*> Deactivate();

	// Loops through systems updating them
	void Update(UpdateContext& ctx);
    
    template<typename Type, eastl::enable_if_t<!eastl::is_base_of<SpatialComponent, Type>::value, int> = 0>
    Type* AddNewComponent(Uuid spatialParent = Uuid())
    {
        Type* pComponent = new Type();
        pComponent->owningEntityId = GetId();
        components.push_back(static_cast<IComponent*>(pComponent));
        return pComponent;
    }

    template<typename Type, eastl::enable_if_t<eastl::is_base_of<SpatialComponent, Type>::value, int> = 0>
    Type* AddNewComponent(Uuid spatialParent = Uuid())
    {
        Type* pComponent = new Type();
        pComponent->owningEntityId = GetId();
        components.push_back(static_cast<IComponent*>(pComponent));

        if (!spatialParent.IsNill())
        {
            eastl::vector<IComponent*>::iterator found = eastl::find_if(components.begin(), components.end(),
            [&spatialParent] (const IComponent* pComp) { return pComp->GetId() == spatialParent; });

            if (found != components.end())
            {
                IComponent* pPotentialParent = *found;
                ASSERT(pPotentialParent->GetTypeData().IsDerivedFrom<SpatialComponent>(), "Attempting to parent to a non spatial component. Not allowed");
                
                SpatialComponent* pSpatial = static_cast<SpatialComponent*>(pComponent);
                pSpatial->SetParent(static_cast<SpatialComponent*>(pPotentialParent));
            }
        }
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