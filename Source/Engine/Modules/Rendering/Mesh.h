#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "GraphicsDevice.h"

#include <EASTL/string.h>
#include <EASTL/vector.h>

struct Vert_PosNormTexCol
{
    Vec3f position;
    Vec3f norm;
    Vec2f texCoord;
    Vec4f color;
};

struct Vert_PosTex
{
    Vec3f position;
    Vec2f texCoord;
};

struct Vert_PosCol
{
    Vec3f position;
    Vec4f color;
};

struct Vert_PosTexCol
{
    Vec3f position;
    Vec2f texCoord;
    Vec4f color;
};

struct Primitive
{
    eastl::vector<Vert_PosNormTexCol> vertBuffer{ nullptr };
    eastl::vector<uint16_t> indexBuffer{ nullptr };

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
