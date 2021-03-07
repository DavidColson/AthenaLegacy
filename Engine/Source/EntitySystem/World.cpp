#include "World.h"

#include "Entity.h"
#include "Systems.h"

Entity* World::NewEntity(eastl::string name)
{
    Entity* pNewEnt = new Entity();
    pNewEnt->name = name;

    entities.push_back(pNewEnt);
    return pNewEnt;
}

void World::ActivateWorld()
{
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

void World::OnUpdate(float deltaTime)
{
    // First update entities
    for (Entity* pEntity : entities)
    {
        pEntity->Update(deltaTime);
    }

    // Then global systems
    for (ISystem* pSystem : globalSystems)
    {
        //pSystem->Update(deltaTime);
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

void World::AddGlobalSystem(ISystem* pSystem)
{
    globalSystems.push_back(pSystem);
}