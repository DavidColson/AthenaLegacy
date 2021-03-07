#include "SpatialComponent.h"

#include "Vec3.h"

REFLECT_BEGIN_DERIVED(SpatialComponent, IComponent)
REFLECT_END()

void SpatialComponent::SetLocalPosition(const Vec3f& position)
{
    Vec3f trans;
    Vec3f rot;
    Vec3f sca;
    localTransform.ToTRS(trans, rot, sca);
    trans = position;
    localTransform = Matrixf::MakeTRS(trans, rot, sca);

    UpdateParentAndChildTransforms();
}

void SpatialComponent::SetLocalRotation(const Vec3f& rotation)
{
    Vec3f trans;
    Vec3f rot;
    Vec3f sca;
    localTransform.ToTRS(trans, rot, sca);
    rot = rotation;
    localTransform = Matrixf::MakeTRS(trans, rot, sca);
    
    UpdateParentAndChildTransforms();
}

void SpatialComponent::SetLocalScale(const Vec3f& scale)
{
    Vec3f trans;
    Vec3f rot;
    Vec3f sca;
    localTransform.ToTRS(trans, rot, sca);
    sca = scale;
    localTransform = Matrixf::MakeTRS(trans, rot, sca);
    
    UpdateParentAndChildTransforms();
}

Matrixf SpatialComponent::GetWorldTransform()
{
    return worldTransform;
}

void SpatialComponent::SetParent(SpatialComponent* pDesiredParent)
{
    pParent = pDesiredParent;
    pParent->children.push_back(this);
    UpdateParentAndChildTransforms();
}

void SpatialComponent::UpdateParentAndChildTransforms()
{
    if (pParent)
    {
        worldTransform = pParent->worldTransform * localTransform;
    }
    else
    {
        worldTransform = localTransform;
    }

    if (!children.empty())
    {
        for (SpatialComponent* child : children)
        {
            child->worldTransform = worldTransform * child->localTransform;
        }
    }
}