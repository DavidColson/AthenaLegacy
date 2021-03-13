#include "World.h"

#include "Entity.h"
#include "Systems.h"

Entity* World::NewEntity(eastl::string name)
{
    Entity* pNewEnt = new Entity();
    pNewEnt->name = name;

    if (!isActive)
        entities.push_back(pNewEnt);
    else
        entitiesToAddQueue.push_back(pNewEnt);

    return pNewEnt;
}

void World::ActivateWorld()
{
    for (ISystem* pGlobalSystem : globalSystems)
    {
        pGlobalSystem->Activate();
    }

    for (Entity* pEntity : entities)
    {
        eastl::vector<IComponent*> comps = pEntity->Activate();
        for (IComponent* pComponent : comps)
        {
            for (ISystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->RegisterComponent(pComponent);
            }
        }
    }
    isActive = true;
}

void World::DeactivateWorld()
{
    for (Entity* pEntity : entities)
    {
        eastl::vector<IComponent*> comps = pEntity->Deactivate();
        for (IComponent* pComponent : comps)
        {
            for (ISystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->UnregisterComponent(pComponent);
            }
        }
    }
}

void World::OnUpdate(UpdateContext& ctx)
{
    // Process entities wanting to be added
    for (Entity* pEntityToAdd : entitiesToAddQueue)
    {
        eastl::vector<IComponent*> comps = pEntityToAdd->Activate();
        for (IComponent* pComponent : comps)
        {
            for (ISystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->RegisterComponent(pComponent);
            }
        }
        entities.push_back(pEntityToAdd);
    }
    entitiesToAddQueue.clear();

    // First update entities
    for (Entity* pEntity : entities)
    {
        pEntity->Update(ctx);
    }

    // Then global systems
    for (ISystem* pSystem : globalSystems)
    {
        pSystem->Update(ctx);
    }
}

void World::DestroyWorld()
{
    // Delete entities
    for(Entity* pEntity : entities)
    {
        delete pEntity;
    }
}