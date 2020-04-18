#include "Scene.h"
int s_componentCounter = 0;

#include "Transform.h"

REFLECT_BEGIN(CName)
REFLECT_MEMBER(name)
REFLECT_END()

REFLECT_BEGIN(CVisibility)
REFLECT_MEMBER(visible)
REFLECT_END()

REFLECT_BEGIN(CTransform)
REFLECT_MEMBER(localPos)
REFLECT_MEMBER(localRot)
REFLECT_MEMBER(localSca)
REFLECT_END()

REFLECT_BEGIN(CParent)
REFLECT_MEMBER(parent)
REFLECT_END()

eastl::map<EntityID, eastl::vector<EntityID>> BuildTransformGraphs(Scene& scene)
{
    eastl::map<EntityID, eastl::vector<EntityID>> map;
    for (EntityID ent : SceneView<CParent, CTransform>(scene))
    {
        CParent* pParent = scene.Get<CParent>(ent);

        // Build a map that maps all the parents to their children
        map[pParent->parent].push_back(ent);
    }
    return map;
}

void TransformHeirarchy(Scene& scene, float deltaTime)
{
    // Phase 1 would be transforming all the parent transforms. We'll take their transform components and build a renderMatrix
    // Not doing this as we don't store the render matrix
    for (EntityID ent : SceneView<CTransform>(scene))
    {
        if (!scene.Has<CParent>(ent))
        {
            CTransform* pTrans = scene.Get<CTransform>(ent);
            pTrans->globalTransform = Matrixf::MakeTRS(pTrans->localPos, pTrans->localRot, pTrans->localSca);
        }
    }

    // Phase 2, build this graph from each node
    eastl::map<EntityID, eastl::vector<EntityID>> trees = BuildTransformGraphs(scene);

    // Phase 2, run through graph
    for (const eastl::pair<EntityID, eastl::vector<EntityID>>& tree : trees)
    {
        CTransform* pParentTrans = scene.Get<CTransform>(tree.first);
        
        for (const EntityID& child : tree.second)
        {
            CTransform* pChildTrans = scene.Get<CTransform>(child);
            Matrixf childMat = Matrixf::MakeTRS(pChildTrans->localPos, pChildTrans->localRot, pChildTrans->localSca);

            pChildTrans->globalTransform = pParentTrans->globalTransform * childMat;         
        }
    }
}

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