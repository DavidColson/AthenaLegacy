#include "IntersectionTests.h"

#include <EASTL/algorithm.h>

bool Intersect::RayAABB(Vec3f rayStart, Vec3f rayDir, AABBf bounds, float& resultDistance, Vec3f& resultIntersect)
{
    resultDistance = 0.0f;
    float distMax = FLT_MAX;

    // One for each axis slab
    for (int i = 0; i < 3; i++)
    {
        if (abs(rayDir[i]) < FLT_EPSILON)
        {
            // Edge case of parallel ray
            if (rayStart[i] < bounds.min[i] || rayStart[i] > bounds.max[i]) return 0; // Not within the slab so counts as intersection
        }
        else
        {
            float invD = 1.0f / rayDir[i];
            float t1 = (bounds.min[i] - rayStart[i]) * invD;
            float t2 = (bounds.max[i] - rayStart[i]) * invD;

            if (t1 > t2)
            {
                float temp = t1;
                t1 = t2;
                t2 = temp;
            }
        
            resultDistance = eastl::max(resultDistance, t1);
            distMax = eastl::min(distMax, t2);

            if (resultDistance > distMax) return false;
        }
    }

    resultIntersect = rayStart + rayDir * resultDistance;
    return true;
}

bool Intersect::RayTriangle(Vec3f rayStart, Vec3f rayDir, Vec3f p0, Vec3f p1, Vec3f p2, float& resultDistance, Vec3f& resultIntersect)
{
    rayDir = rayDir.GetNormalized();
    Vec3f edge1 = p1 - p0;
    Vec3f edge2 = p2 - p0;

    Vec3f q = Vec3f::Cross(rayDir, edge2);
    float determinant = Vec3f::Dot(edge1, q);

    if (abs(determinant) < FLT_EPSILON)
        return false;

    float invDet = 1.0f / determinant;

    // u is first barycentric coordinate
    Vec3f s = rayStart - p0;
    float u = invDet * Vec3f::Dot(s, q);
    if (u < 0.0f || u > 1.0f)
        return false;

    // v is second barycentric coordinate
    Vec3f r = Vec3f::Cross(s, edge1);
    float v = invDet * Vec3f::Dot(rayDir, r);
    if (v < 0.0f || (u + v) > 1.0f)
        return false;

    resultDistance = invDet * Vec3f::Dot(edge2, r);
    resultIntersect = rayStart + rayDir * resultDistance;
    return true;
}