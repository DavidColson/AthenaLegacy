#pragma once

#include <Vec3.h>
#include <AABB.h>
#include <EASTL/algorithm.h>

bool IntersectRayAABB(Vec3f rayStart, Vec3f rayDir, AABBf bounds, float& resultDistance, Vec3f& resultIntersect)
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