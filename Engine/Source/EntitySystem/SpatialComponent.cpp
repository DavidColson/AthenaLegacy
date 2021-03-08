#include "SpatialComponent.h"

#include "Vec3.h"

REFLECT_BEGIN_DERIVED(SpatialComponent, IComponent)
REFLECT_END()

void SpatialComponent::SetLocalPosition(const Vec3f& _position)
{
    position = _position;
    UpdateTransforms();
}

Vec3f SpatialComponent::GetLocalPosition()
{
    return position;
}

void SpatialComponent::SetLocalRotation(const Vec3f& _rotation)
{
    rotation = _rotation;
    UpdateTransforms();
}

Vec3f SpatialComponent::GetLocalRotation()
{
    return rotation;
}

void SpatialComponent::SetLocalScale(const Vec3f& _scale)
{
    scale = _scale;
    UpdateTransforms();
}

Vec3f SpatialComponent::GetLocalScale()
{
    return scale;
}

Matrixf SpatialComponent::GetWorldTransform()
{
    return worldTransform;
}

void SpatialComponent::SetParent(SpatialComponent* pDesiredParent)
{
    // TODO: Check to make sure the desired parent even belongs to this entity
    pParent = pDesiredParent;
    pParent->children.push_back(this);
    UpdateTransforms();
}

void SpatialComponent::UpdateTransforms()
{
    localTransform = Matrixf::MakeTRS(position, rotation, scale);

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