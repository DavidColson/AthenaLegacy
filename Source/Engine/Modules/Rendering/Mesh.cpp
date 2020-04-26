#include "Mesh.h"

void Mesh::CreateGfxDeviceBuffers()
{
    for(Primitive& prim : primitives)
    {
        prim.gfxVertBuffer = GfxDevice::CreateVertexBuffer(prim.vertBuffer.size(), sizeof(Vert_PosNormTexCol), prim.vertBuffer.data(), "PrimitiveVertBuffer");
        prim.gfxIndexBuffer = GfxDevice::CreateIndexBuffer(prim.indexBuffer.size(), IndexFormat::UShort, prim.indexBuffer.data(), "PrimitiveIndexBuffer");
    }
}