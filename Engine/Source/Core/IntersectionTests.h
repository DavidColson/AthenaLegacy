#pragma once

#include <Vec3.h>
#include <AABB.h>

namespace Intersect
{

bool RayAABB(Vec3f rayStart, Vec3f rayDir, AABBf bounds, float& resultDistance, Vec3f& resultIntersect);
bool RayTriangle(Vec3f rayStart, Vec3f rayDir, Vec3f p0, Vec3f p1, Vec3f p2, float& resultDistance, Vec3f& resultIntersect);

}