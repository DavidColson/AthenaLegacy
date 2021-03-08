#pragma once

#include "Entity.h"
#include "Matrix.h"

struct SpatialComponent : public IComponent
{
    REFLECT_DERIVED()

    void SetLocalPosition(const Vec3f& position);

    void SetLocalRotation(const Vec3f& rotation);

    void SetLocalScale(const Vec3f& scale);

    Matrixf GetWorldTransform();

    void SetParent(SpatialComponent* pDesiredParent);

private:
    Matrixf localTransform{ Matrixf::Identity() };
    Matrixf worldTransform{ Matrixf::Identity() };

    void UpdateParentAndChildTransforms();

    SpatialComponent* pParent{ nullptr };
    eastl::vector<SpatialComponent*> children;
};
