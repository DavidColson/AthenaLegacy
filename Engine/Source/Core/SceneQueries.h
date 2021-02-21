#pragma once

#include "Vec3.h"
#include "Scene.h"

namespace SceneQueries
{

struct Hit
{
    EntityID ent{ EntityID::InvalidID() };
    Vec3f hitLoc{ Vec3f(0.0f) };
    float distance{ 0.0f };
};

bool RaycastRenderables(Scene& scene, Vec3f rayStart, Vec3f rayEnd, Hit& outHit);

}