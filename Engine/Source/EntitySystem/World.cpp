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

void World::DestroyEntity(Entity* pEntity)
{
    entitiesToDeleteQueue.push_back(pEntity);
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
    isActive = false;
}

void World::OnUpdate(UpdateContext& ctx)
{
    // Process entities wanting to be deleted
    for (Entity* pEntityToDelete : entitiesToDeleteQueue)
    {
        eastl::vector<IComponent*> comps = pEntityToDelete->Deactivate();
        for (IComponent* pComponent : comps)
        {
            for (IWorldSystem* pGlobalSystem : globalSystems)
            {
                pGlobalSystem->UnregisterComponent(pEntityToDelete, pComponent);
            }
        }
        eastl::vector<Entity*>::iterator found = eastl::find(entities.begin(), entities.end(), pEntityToDelete);
        if (found != entities.end())
        {
            entities.erase(found);
        }
    }
    entitiesToDeleteQueue.clear();

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
    if (isActive)
        DeactivateWorld();
        
    // Delete entities
    for(Entity* pEntity : entities)
    {
        delete pEntity;
    }
}