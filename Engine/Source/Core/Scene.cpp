#include "Scene.h"
eastl::map<uint32_t, uint32_t> s_componentTypeIdMap;

REFLECT_COMPONENT_BEGIN(CName)
REFLECT_MEMBER(name)
REFLECT_END()

REFLECT_COMPONENT_BEGIN(CVisibility)
REFLECT_MEMBER(visible)
REFLECT_END()

REFLECT_COMPONENT_BEGIN(CTransform)
REFLECT_MEMBER(localPos)
REFLECT_MEMBER(localRot)
REFLECT_MEMBER(localSca)
REFLECT_END()

REFLECT_COMPONENT_BEGIN(CParent)
REFLECT_MEMBER(nChildren)
REFLECT_MEMBER(firstChild)
REFLECT_END()

REFLECT_COMPONENT_BEGIN(CChild)
REFLECT_MEMBER(parent)
REFLECT_MEMBER(prev)
REFLECT_MEMBER(next)
REFLECT_END()

// ***********************************************************************

void RecursiveTransformTree(Scene& scene, EntityID root)
{
    CTransform* pParentTrans = scene.Get<CTransform>(root);
    CParent* pParent = scene.Get<CParent>(root);
			
	EntityID currChild = pParent->firstChild;
	for(int i = 0; i < pParent->nChildren; i++)
	{
        if (!scene.Has<CTransform>(currChild))
            continue;

        CTransform* pChildTrans = scene.Get<CTransform>(currChild);

        Matrixf childMat = Matrixf::MakeTRS(pChildTrans->localPos, pChildTrans->localRot, pChildTrans->localSca);
        pChildTrans->globalTransform = pParentTrans->globalTransform * childMat;

        if (scene.Has<CParent>(currChild))
            RecursiveTransformTree(scene, currChild);

		currChild = scene.Get<CChild>(currChild)->next;
    }
}

// ***********************************************************************

void TransformHeirarchy(Scene& scene, float deltaTime)
{
    // Phase 1 would be transforming all the parent transforms. We'll take their transform components and build a renderMatrix
    // Not doing this as we don't store the render matrix
    eastl::vector<EntityID> roots;

    for (EntityID ent : SceneIterator<CTransform>(scene))
    {   
        // We only want root entities here, so ignoring children, they'll come later
        if (!scene.Has<CChild>(ent))
        {
            CTransform* pTrans = scene.Get<CTransform>(ent);
            pTrans->globalTransform = Matrixf::MakeTRS(pTrans->localPos, pTrans->localRot, pTrans->localSca);

            if (scene.Has<CParent>(ent)) // If this entity is the top of a tree, we need to recurse it's children
                roots.push_back(ent);
        }
    }

    // Phase 2, run through each roots graph updating all their children
    for (EntityID root : roots)
    {
        RecursiveTransformTree(scene, root);
    }
}

// ***********************************************************************

ComponentPool::ComponentPool(size_t elementsize, TypeData& typeData, void(*_pDestructor)(void*, TypeData*))
{
    elementSize = elementsize;
    pData = new char[elementSize * MAX_ENTITIES];
    pDestructor = _pDestructor;
    pTypeData = &typeData;
}

// ***********************************************************************

ComponentPool::~ComponentPool()
{
    delete[] pData;
}

// ***********************************************************************

Scene::Scene()
{
    Engine::NewSceneCreated(*this);
}

// ***********************************************************************

Scene::~Scene()
{
    for (EntityDesc& desc : entities)
    {
        DestroyEntity(desc.id);
    }
    for (ComponentPool* pPool : componentPools)
    {
        delete pPool;
    }
}

// ***********************************************************************

EntityID Scene::NewEntity(const char* name)
{
    nActiveEntities++;
    if (!freeEntities.empty())
    {
        EntityIndex newIndex = freeEntities.back();
        freeEntities.pop_back();
        entities[newIndex].id = EntityID::New(newIndex, entities[newIndex].id.Version());
        Assign<CName>(entities[newIndex].id)->name = name;
        return entities[newIndex].id;
    }
    entities.push_back({ EntityID::New(EntityIndex(entities.size()), 0), ComponentMask() });
    Assign<CName>(entities.back().id)->name = name;
    return entities.back().id;
}

// ***********************************************************************

void Scene::DestroyEntity(EntityID id)
{
    if (!id.IsValid())
        return;

    for (ComponentPool* pPool : componentPools)
    {
        if (pPool != nullptr && pPool->pTypeData->IsValid() && Has(id, *(pPool->pTypeData)))
        {
            for (ReactiveSystemFunc func : pPool->onRemovedCallbacks)
            {
                func(*this, id);
            }
            pPool->Erase(id.Index());
        }
    }
    entities[id.Index()].id = EntityID::New(EntityIndex(-1), id.Version() + 1); // set to invalid
    entities[id.Index()].mask.reset(); // clear components
    freeEntities.push_back(id.Index());
    nActiveEntities--;
}

// ***********************************************************************

eastl::string Scene::GetEntityName(EntityID entity)
{
    return Get<CName>(entity)->name;
}

// ***********************************************************************

void Scene::SetParent(EntityID child, EntityID parent)
{
    // Ensure they have relationship components
    if (!Has<CParent>(parent))
        Assign<CParent>(parent);

    if (!Has<CChild>(child))
        Assign<CChild>(child);

    
    CParent* pParent = Get<CParent>(parent);
    // First case, this is the first child being attached to this parent
    if (pParent->firstChild == EntityID::InvalidID())
    {
        pParent->firstChild = child;

        CChild* pNewChild = Get<CChild>(child);
        pNewChild->parent = parent;
    }
    else
    {
        EntityID current = pParent->firstChild;
        for (int i = 0; i < pParent->nChildren - 1; i++)
            current = Get<CChild>(current)->next;

        CChild* pNewChild = Get<CChild>(child);
        pNewChild->prev = current;
        pNewChild->parent = parent;
        
        Get<CChild>(current)->next = child;
    }
    pParent->nChildren += 1;
}

// ***********************************************************************

void Scene::UnsetParent(EntityID child, EntityID parent)
{
    CChild* pChild = Get<CChild>(child);
    CParent* pParent = Get<CParent>(parent);

    // This is the only child of this parent
    if (pChild->prev == EntityID::InvalidID() && pChild->next == EntityID::InvalidID())
    {
        pParent->firstChild = EntityID::InvalidID();
        Get<CParent>(parent)->firstChild = pChild->next;
    }
    // This case means this is the first child, but not the only one
    else if (pChild->prev == EntityID::InvalidID() && pChild->next != EntityID::InvalidID())
    {
        pParent->firstChild = pChild->next;
    }
    // This is a normal case of a child among many
    else
    {
        Get<CChild>(pChild->prev)->next = pChild->next;
    }
    
    Remove<CChild>(child);
    pParent->nChildren -= 1;
}

// ***********************************************************************

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

// ***********************************************************************

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

// ***********************************************************************

void Scene::RenderScene(float deltaTime)
{
    for (SystemFunc func : renderSystems)
    {
        func(*this, deltaTime);
    }
}

// ***********************************************************************

void Scene::Assign(EntityID id, TypeData& componentType)
{
    if (entities[id.Index()].id != id)
        return;

    ComponentPool *pPool = GetOrCreateComponentPool(componentType);
    ASSERT(Has(id, componentType) == false, "You're trying to assign a component to an entity that already has this component");

    componentType.pTypeOps->PlacementNew(pPool->GetRaw(id.Index()));

    entities[id.Index()].mask.set(componentType.id);

    for (ReactiveSystemFunc func : pPool->onAddedCallbacks)
    {
        func(*this, id);
    }
}

// ***********************************************************************

void Scene::Remove(EntityID id, TypeData& componentType)
{
    if (entities[id.Index()].id != id)
        return;

    ASSERT(Has(id, componentType), "The component you're trying to access is not assigned to this entity");

    int componentId = componentType.id;
    for (ReactiveSystemFunc func : componentPools[componentId]->onRemovedCallbacks)
    {
        func(*this, id);
    }
    entities[id.Index()].mask.reset(componentId);
    componentPools[componentId]->Erase(id.Index());
}

// ***********************************************************************

Variant Scene::Get(EntityID id, TypeData& componentType)
{
    if (entities[id.Index()].id != id)
        return nullptr;

    ASSERT(Has(id, componentType), "The component you're trying to access is not assigned to this entity");

    void* pData = componentPools[componentType.id]->GetRaw(id.Index());
    return componentType.pTypeOps->CopyToVariant(pData);
}

// ***********************************************************************

void Scene::Set(EntityID id, Variant componentToSet)
{
    if (entities[id.Index()].id != id)
        return;
    
    int componentId = componentToSet.GetType().id;

    ASSERT(Has(id, componentToSet.GetType()), "The component you're trying to set data on is not assigned to this entity");

    void* pData = componentPools[componentId]->GetRaw(id.Index());
    componentToSet.GetType().pTypeOps->Copy(pData, componentToSet.pData);
}

// ***********************************************************************

bool Scene::Has(EntityID id, TypeData &type)
{
    if (!id.IsValid() || entities[id.Index()].id != id) // ensures you're not accessing an entity that has been deleted
        return false;

    return entities[id.Index()].mask.test(type.id);
}

// ***********************************************************************

void Scene::RegisterReactiveSystem(Reaction reaction, ReactiveSystemFunc func, TypeData& type)
{
    ComponentPool *pPool = GetOrCreateComponentPool(type);

    switch (reaction)
    {
    case Reaction::OnAdd:
        pPool->onAddedCallbacks.push_back(func);
        break;
    case Reaction::OnRemove:
        pPool->onRemovedCallbacks.push_back(func);
        break;
    default:
        break;
    }
}

// ***********************************************************************

ComponentPool* Scene::GetOrCreateComponentPool(TypeData& type)
{
    uint32_t componentId = type.id;
    if (componentPools.size() <= componentId) // Not enough component pool
        componentPools.resize(componentId + 1, nullptr);

    if (componentPools[componentId] == nullptr) // New component, make a new pool
    {	
        componentPools[componentId] = new ComponentPool(type.size, type, [](void *pComponent, TypeData* pTypeData) {
            pTypeData->pTypeOps->Destruct(pComponent);
        });
    }
    return componentPools[componentId];
}