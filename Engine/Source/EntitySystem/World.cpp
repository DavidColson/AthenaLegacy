#include "World.h"

#include "Entity.h"
#include "Systems.h"



World::~World()
{
    if (isActive)
        DeactivateWorld();

    // Delete entities
    for(Entity* pEntity : entities)
    {
        delete pEntity;
    }

    for(IWorldSystem* pSystem : globalSystems)
    {
        delete pSystem;
    }
}

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

void World::DestroyEntity(Uuid entityId)
{
    eastl::vector<Entity*>::iterator found = eastl::find_if(entities.begin(), entities.end(), [&entityId](const Entity* pEntity) { return entityId == pEntity->GetId(); });
    if (found != entities.end())
    {
        entitiesToDeleteQueue.push_back(*found);
    }
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

    for (IWorldSystem* pGlobalSystem : globalSystems)
    {
        pGlobalSystem->Deactivate();
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

// ***********************************************************************

Entity* World::FindEntity(Uuid entityId)
{
    eastl::vector<Entity*>::iterator found = eastl::find_if(entities.begin(), entities.end(), [&entityId](const Entity* pEntity) { return entityId == pEntity->GetId(); });
    if (found != entities.end())
    {
        return *found;
    }
    return nullptr;
}

// ***********************************************************************

Entity* World::EntityIterator::operator*() const 
{ 
	return *it;
}

// ***********************************************************************

bool World::EntityIterator::operator==(const EntityIterator& other) const 
{
	return it == other.it;
}

// ***********************************************************************

bool World::EntityIterator::operator!=(const EntityIterator& other) const 
{
	return it != other.it;
}

// ***********************************************************************

World::EntityIterator& World::EntityIterator::operator++()
{
	++it;
	return *this;
}

// ***********************************************************************

const World::EntityIterator World::begin() 
{
	return EntityIterator(entities.begin());
}

// ***********************************************************************

const World::EntityIterator World::end()
{
	return EntityIterator(entities.end());
}

