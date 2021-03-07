#include "Entity.h"

#include "Systems.h"


REFLECT_BEGIN(IComponent)
REFLECT_END()

eastl::vector<IComponent*> Entity::Activate()
{
    eastl::vector<IComponent*> componentsToReturn;
    for (IComponent* pComponent : components)
    {
        componentsToReturn.push_back(pComponent);
        for (ISystem* pSystem : systems)
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
        for (ISystem* pSystem : systems)
        {
            pSystem->UnregisterComponent(pComponent);
        }
    }
    return componentsToReturn;
}

void Entity::Update(float deltaTime)
{
    for (ISystem* pSystem : systems)
    {
        //pSystem->Update(deltaTime);
    }
}