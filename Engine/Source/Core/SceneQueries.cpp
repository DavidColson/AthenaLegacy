#include "SceneQueries.h"

#include "Rendering/SceneDrawSystem.h"
#include "AssetDatabase.h"
#include "Mesh.h"
#include "IntersectionTests.h"

bool SceneQueries::RaycastRenderables(Scene& scene, Vec3f rayStart, Vec3f rayDir, Hit& outHit)
{
    for (EntityID ent : SceneIterator<CRenderable, CTransform>(scene))
    {
        CRenderable* pRenderable = scene.Get<CRenderable>(ent);
        CTransform* pTrans = scene.Get<CTransform>(ent);
        Mesh* pMesh = AssetDB::GetAsset<Mesh>(pRenderable->meshHandle);

        for (Primitive& prim : pMesh->primitives)
        {
            float dist = 0;
            Vec3f hitLoc;

            // TODO: Dave, these translations are incorrect somehow I guarantee it
            Vec3f localRayStart = pTrans->globalTransform.GetInverse() * rayStart;

            Matrixf withoutTrans = pTrans->globalTransform;
            withoutTrans.m[0][3] = 0.0f;
            withoutTrans.m[1][3] = 0.0f;
            withoutTrans.m[2][3] = 0.0f;

            Vec3f localRayDir = withoutTrans.GetInverse() * rayDir;

			AABBf worldBounds = TransformAABB(prim.localBounds, pTrans->globalTransform);
            if (Intersect::RayAABB(rayStart, rayDir, worldBounds, dist, hitLoc))
            {
                // TODO: This may not be the closest hit. Might want to put results in a temp vector and sort by distance?
                // TODO: This will get slow with large number of polys. Need to use a BVH for faster raycasts

                // Okay, we know it might be this primitive. Need to now check the polys individually    
                if (prim.topologyType == TopologyType::TriangleList)
                {
                    // Easy iterate 3 at a time, next three elements all make up one triangle
                    for (size_t i = 0; i < prim.indices.size(); i+=3)
                    {
                        Vec3f p0 = prim.vertices[prim.indices[i]];   
                        Vec3f p1 = prim.vertices[prim.indices[i+1]];   
                        Vec3f p2 = prim.vertices[prim.indices[i+2]];

                        if (Intersect::RayTriangle(localRayStart, localRayDir, p0, p1, p2, dist, hitLoc))
                        {
                            // We've hit this entity!
                            outHit = Hit{ent, pTrans->globalTransform * hitLoc, dist};
                            return true;
                        }
                    }
                }
                else if (prim.topologyType == TopologyType::TriangleStrip)
                {
                    // Triangle Strips are awkward as fuck
                    for (size_t i = 0; i < prim.indices.size() - 2; i++)
                    {
                        Vec3f p0, p1, p2;   
                        if (i % 2 == 0)
                        {
                            p0 = prim.vertices[prim.indices[i]];   
                            p1 = prim.vertices[prim.indices[i+1]];   
                            p2 = prim.vertices[prim.indices[i+2]];
                        }
                        else
                        {
                            p0 = prim.vertices[prim.indices[i]];   
                            p1 = prim.vertices[prim.indices[i+2]];   
                            p2 = prim.vertices[prim.indices[i+1]];
                        }
                        if (Intersect::RayTriangle(localRayStart, localRayDir, p0, p1, p2, dist, hitLoc))
                        {
                            // We've hit this entity!
                            outHit = Hit{ent, pTrans->globalTransform * hitLoc, dist};
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
} 