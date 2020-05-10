#include "Mesh.h"

#include "AssetDatabase.h"

void Mesh::Load(eastl::string path)
{
    // Doesn't actually load anything from disk since this would be a subasset of a model
    // If (AssetDB::IsSubasset(handle))
    // bla bla

    AssetDB::RegisterAsset(this, path);
    CreateGfxDeviceBuffers();
}

void Mesh::CreateGfxDeviceBuffers()
{
    for(Primitive& prim : primitives)
    {
        prim.gfxVertBuffer = GfxDevice::CreateVertexBuffer(prim.vertBuffer.size(), sizeof(Vert_PosNormTexCol), prim.vertBuffer.data(), name + " vBuffer");
        prim.gfxIndexBuffer = GfxDevice::CreateIndexBuffer(prim.indexBuffer.size(), IndexFormat::UShort, prim.indexBuffer.data(), name + " iBuffer");
    }
}