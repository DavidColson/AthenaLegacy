#pragma once

#include "Entity.h"
#include "Matrix.h"

struct SpatialComponent : public IComponent
{
    friend class Entity;

    REFLECT_DERIVED()

    SpatialComponent() : IComponent() {}

    void SetLocalPosition(const Vec3f& position);

    Vec3f GetLocalPosition();


    void SetLocalRotation(const Vec3f& rotation);

    Vec3f GetLocalRotation();


    void SetLocalScale(const Vec3f& scale);

    Vec3f GetLocalScale();


    Matrixf GetWorldTransform();

private:
    void SetParent(SpatialComponent* pDesiredParent);

    Vec3f position{ Vec3f(0.0f) };
    Vec3f rotation{ Vec3f(0.0f) };
    Vec3f scale{ Vec3f(1.0f) };
    Matrixf localTransform{ Matrixf::Identity() };
    Matrixf worldTransform{ Matrixf::Identity() };

    void UpdateTransforms();

    SpatialComponent* pParent{ nullptr };
    eastl::vector<SpatialComponent*> children;
};
