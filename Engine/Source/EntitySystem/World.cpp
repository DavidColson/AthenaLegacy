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
    for (IWorldSystem* pGlobalSystem : globalSystems)
    {
        pGlobalSystem->Activate();
    }

    for (Entity* pEntity : entities)
    {
        eastl::vector<IComponent*> comps = pEntity->Activate();
        for (IComponent* pComponent : comps)
        {
            for (IWorldSystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->RegisterComponent(pEntity, pComponent);
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
            for (IWorldSystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->UnregisterComponent(pEntity, pComponent);
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
            for (IWorldSystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->RegisterComponent(pEntityToAdd, pComponent);
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
    for (IWorldSystem* pSystem : globalSystems)
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