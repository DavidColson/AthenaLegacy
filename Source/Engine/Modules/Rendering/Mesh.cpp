#include "Mesh.h"

void Mesh::CreateGfxDeviceBuffers()
{
    for(Primitive& prim : primitives)
    {
        prim.gfxVertBuffer = GfxDevice::CreateVertexBuffer(prim.nVerts, sizeof(VertPos), prim.pVertBuffer, "PrimitiveVertBuffer");
        prim.gfxIndexBuffer = GfxDevice::CreateIndexBuffer(prim.nIndices, IndexFormat::UShort, prim.pIndexBuffer, "PrimitiveIndexBuffer");
    }
}