#pragma once

#include "Vec3.h"
#include "GraphicsDevice.h"

#include <EASTL/string.h>
#include <EASTL/vector.h>

struct VertPos
{
    Vec3f position;
};

struct Primitive
{
    VertPos* pVertBuffer{ nullptr };
    int nVerts{ 0 };

    unsigned short* pIndexBuffer{ nullptr };
    int nIndices{ 0 };

    TopologyType topologyType{TopologyType::TriangleList};
    VertexBufferHandle gfxVertBuffer;
    IndexBufferHandle gfxIndexBuffer;
};

struct Mesh
{
    eastl::string name;
    eastl::vector<Primitive> primitives;

    void CreateGfxDeviceBuffers();
};
