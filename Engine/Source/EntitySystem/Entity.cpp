#include "Entity.h"

#include "Systems.h"

REFLECT_BEGIN(IComponent)
REFLECT_END()

Entity::~Entity()
{
    // Delete entities
    for(IComponent* pComponent : components)
    {
        delete pComponent;
    }

    for(IEntitySystem* pSystem : systems)
    {
        delete pSystem;
    }
}

eastl::vector<IComponent*> Entity::Activate()
{
    eastl::vector<IComponent*> componentsToReturn;
    for (IComponent* pComponent : components)
    {
        componentsToReturn.push_back(pComponent);
        for (IEntitySystem* pSystem : systems)
        {
            pSystem->RegisterComponent(pComponent);
        }
    }
    return componentsToReturn;
}

eastl::vector<IComponent*> Entity::Deactivate()
{
    eastl::vector<IComponent*> componentsToReturn;
    for (IComponent* pComponent : components)
    {
        componentsToReturn.push_back(pComponent);
        for (IEntitySystem* pSystem : systems)
        {
            pSystem->UnregisterComponent(pComponent);
        }
    }
    return componentsToReturn;
}

void Entity::Update(UpdateContext& ctx)
{
    for (IEntitySystem* pSystem : systems)
    {
        pSystem->Update(ctx);
    }
}