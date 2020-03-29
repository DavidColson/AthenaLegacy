#include "Scene.h"
int s_componentCounter = 0;

REFLECT_BEGIN(CName)
REFLECT_MEMBER(name)
REFLECT_END()

REFLECT_BEGIN(CVisibility)
REFLECT_MEMBER(visible)
REFLECT_END()

REFLECT_BEGIN(CTransform)
REFLECT_MEMBER(pos)
REFLECT_MEMBER(rot)
REFLECT_MEMBER(sca)
REFLECT_MEMBER(vel)
REFLECT_END()

BaseComponentPool::BaseComponentPool(size_t elementsize)
{
    elementSize = elementsize;
    pData = new char[elementSize * MAX_ENTITIES];
}

BaseComponentPool::~BaseComponentPool()
{
    delete[] pData;
}

Scene::Scene()
{
    Engine::NewSceneCreated(*this);
}

Scene::~Scene()
{
    for (EntityDesc& desc : entities)
    {
        DestroyEntity(desc.id);
    }
    for (BaseComponentPool* pPool : componentPools)
    {
        delete pPool;
    }
}

EntityID Scene::NewEntity(const char* name)
{
    nActiveEntities++;
    if (!freeEntities.empty())
    {
        EntityIndex newIndex = freeEntities.back();
        freeEntities.pop_back();
        entities[newIndex].id = CreateEntityId(newIndex, GetEntityVersion(entities[newIndex].id));
        Assign<CName>(entities[newIndex].id)->name = name;
        return entities[newIndex].id;
    }
    entities.push_back({ CreateEntityId(EntityIndex(entities.size()), 0), ComponentMask() });
    Assign<CName>(entities.back().id)->name = name;
    return entities.back().id;
}

void Scene::DestroyEntity(EntityID id)
{
    if (!IsEntityValid(id))
        return;

    for (int i = 0; i < MAX_COMPONENTS; i++)
    {
        // For each component ID, check the bitmask, if no, continue, if yes, destroy the component
        eastl::bitset<MAX_COMPONENTS> mask;
        mask.set(i, true);
        if (mask == (entities[GetEntityIndex(id)].mask & mask))
        {
            for (ReactiveSystemFunc func : componentPools[i]->onRemovedCallbacks)
            {
                func(*this, id);
            }
            componentPools[i]->destroy(GetEntityIndex(id));
        }
    }
    entities[GetEntityIndex(id)].id = CreateEntityId(EntityIndex(-1), GetEntityVersion(id) + 1); // set to invalid
    entities[GetEntityIndex(id)].mask.reset(); // clear components
    freeEntities.push_back(GetEntityIndex(id));
    nActiveEntities--;
}

eastl::string Scene::GetEntityName(EntityID entity)
{
    return Get<CName>(entity)->name;
}

void Scene::RegisterSystem(SystemPhase phase, SystemFunc func)
{
    switch (phase)
    {
    case SystemPhase::PreUpdate:
        preUpdateSystems.push_back(func);
        break;			
    case SystemPhase::Update:
        updateSystems.push_back(func);
        break;
    case SystemPhase::Render:
        renderSystems.push_back(func);
        break;
    default:
        break;
    }
}

void Scene::SimulateScene(float deltaTime)
{
    for (SystemFunc func : preUpdateSystems)
    {
        func(*this, deltaTime);
    }
    for (SystemFunc func : updateSystems)
    {
        func(*this, deltaTime);
    }
}

void Scene::RenderScene(float deltaTime)
{
    for (SystemFunc func : renderSystems)
    {
        func(*this, deltaTime);
    }
}