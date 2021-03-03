#include "Mesh.h"

#include "AssetDatabase.h"

void Mesh::Load(Path path, AssetHandle handleForThis)
{
    // Doesn't actually load anything from disk since this would be a subasset of a model
    // If (AssetDB::IsSubasset(handle))
    // bla bla
    CreateGfxBuffers();
}

void Mesh::CreateGfxBuffers()
{
    for(Primitive& prim : primitives)
    {
       prim.CreateGfxBuffers();
    }
}

Primitive::Primitive(const Primitive& copy)
{
    vertices = eastl::vector<Vec3f>(copy.vertices);
    normals = eastl::vector<Vec3f>(copy.normals);
    uv0 = eastl::vector<Vec2f>(copy.uv0);
    uvz0 = eastl::vector<Vec3f>(copy.uvz0);
    uvz1 = eastl::vector<Vec3f>(copy.uvz1);
    uvzw0 = eastl::vector<Vec4f>(copy.uvzw0);
    colors = eastl::vector<Vec4f>(copy.colors);
    indices = eastl::vector<uint16_t>(copy.indices);
    topologyType = copy.topologyType;
    localBounds = copy.localBounds;

    CreateGfxBuffers();
}

Primitive::Primitive(Primitive&& copy)
{
    vertices = copy.vertices;
    normals = copy.normals;
    uv0 = copy.uv0;
    uvz0 = copy.uvz0;
    uvz1 = copy.uvz1;
    uvzw0 = copy.uvzw0;
    colors = copy.colors;
    indices = copy.indices;
    topologyType = copy.topologyType;
    localBounds = copy.localBounds;

    bufferHandle_vertices = copy.bufferHandle_vertices;
    bufferHandle_normals = copy.bufferHandle_normals;
    bufferHandle_uv0 = copy.bufferHandle_uv0;
    bufferHandle_uvz0 = copy.bufferHandle_uvz0;
    bufferHandle_uvz1 = copy.bufferHandle_uvz1;
    bufferHandle_uvzw0 = copy.bufferHandle_uvzw0;
    bufferHandle_colors = copy.bufferHandle_colors;
    bufferHandle_indices = copy.bufferHandle_indices;

    copy.bufferHandle_vertices = INVALID_HANDLE;
    copy.bufferHandle_normals = INVALID_HANDLE;
    copy.bufferHandle_uv0 = INVALID_HANDLE;
    copy.bufferHandle_uvz0 = INVALID_HANDLE;
    copy.bufferHandle_uvz1 = INVALID_HANDLE;
    copy.bufferHandle_uvzw0 = INVALID_HANDLE;
    copy.bufferHandle_colors = INVALID_HANDLE;
    copy.bufferHandle_indices = INVALID_HANDLE;
}

Primitive& Primitive::operator=(const Primitive& copy)
{
    GfxDevice::FreeVertexBuffer(bufferHandle_vertices);
    GfxDevice::FreeVertexBuffer(bufferHandle_normals);
    GfxDevice::FreeVertexBuffer(bufferHandle_uv0);
    GfxDevice::FreeVertexBuffer(bufferHandle_uvz0);
    GfxDevice::FreeVertexBuffer(bufferHandle_uvz1);
    GfxDevice::FreeVertexBuffer(bufferHandle_uvzw0);
    GfxDevice::FreeVertexBuffer(bufferHandle_colors);
    GfxDevice::FreeIndexBuffer(bufferHandle_indices);

    vertices = eastl::vector<Vec3f>(copy.vertices);
    normals = eastl::vector<Vec3f>(copy.normals);
    uv0 = eastl::vector<Vec2f>(copy.uv0);
    uvz0 = eastl::vector<Vec3f>(copy.uvz0);
    uvz1 = eastl::vector<Vec3f>(copy.uvz1);
    uvzw0 = eastl::vector<Vec4f>(copy.uvzw0);
    colors = eastl::vector<Vec4f>(copy.colors);
    indices = eastl::vector<uint16_t>(copy.indices);
    topologyType = copy.topologyType;
    localBounds = copy.localBounds;
    CreateGfxBuffers();
    
    return *this;
}

Primitive& Primitive::operator=(Primitive&& copy)
{
    vertices = copy.vertices;
    normals = copy.normals;
    uv0 = copy.uv0;
    uvz0 = copy.uvz0;
    uvz1 = copy.uvz1;
    uvzw0 = copy.uvzw0;
    colors = copy.colors;
    indices = copy.indices;
    topologyType = copy.topologyType;

    localBounds = copy.localBounds;

    bufferHandle_vertices = copy.bufferHandle_vertices;
    bufferHandle_normals = copy.bufferHandle_normals;
    bufferHandle_uv0 = copy.bufferHandle_uv0;
    bufferHandle_uvz0 = copy.bufferHandle_uvz0;
    bufferHandle_uvz1 = copy.bufferHandle_uvz1;
    bufferHandle_uvzw0 = copy.bufferHandle_uvzw0;
    bufferHandle_colors = copy.bufferHandle_colors;
    bufferHandle_indices = copy.bufferHandle_indices;

    copy.bufferHandle_vertices = INVALID_HANDLE;
    copy.bufferHandle_normals = INVALID_HANDLE;
    copy.bufferHandle_uv0 = INVALID_HANDLE;
    copy.bufferHandle_uvz0 = INVALID_HANDLE;
    copy.bufferHandle_uvz1 = INVALID_HANDLE;
    copy.bufferHandle_uvzw0 = INVALID_HANDLE;
    copy.bufferHandle_colors = INVALID_HANDLE;
    copy.bufferHandle_indices = INVALID_HANDLE;

    return *this;
}

Primitive::~Primitive()
{
    GfxDevice::FreeVertexBuffer(bufferHandle_vertices);
    GfxDevice::FreeVertexBuffer(bufferHandle_normals);
    GfxDevice::FreeVertexBuffer(bufferHandle_uv0);
    GfxDevice::FreeVertexBuffer(bufferHandle_uvz0);
    GfxDevice::FreeVertexBuffer(bufferHandle_uvz1);
    GfxDevice::FreeVertexBuffer(bufferHandle_uvzw0);
    GfxDevice::FreeVertexBuffer(bufferHandle_colors);
    GfxDevice::FreeIndexBuffer(bufferHandle_indices);
}

void Primitive::RecalcLocalBounds()
{
    // Loop through vertices, collecitng the max in each axis.
    for (int i = 0; i < 3; i++)
    {
        Vec3f dir;
        dir[i] = 1.0f;

        float minProj = FLT_MAX; float maxProj = -FLT_MAX;
        int vertMin; int vertMax;
        for (int n = 0; n < (int)vertices.size(); n++)
        {
            float projection = Vec3f::Dot(vertices[n], dir);
            if (projection < minProj)
            {
                minProj = projection;
                vertMin = n;
            }

            if (projection > maxProj)
            {
                maxProj = projection;
                vertMax = n;
            }
        }
        localBounds.min[i] = vertices[vertMin][i];
        localBounds.max[i] = vertices[vertMax][i];
    }
}

void Primitive::CreateGfxBuffers()
{
    if (!vertices.empty()) bufferHandle_vertices = GfxDevice::CreateVertexBuffer(vertices.size(), sizeof(Vec3f), vertices.data(), name + " vertex position buffer");
    if (!normals.empty()) bufferHandle_normals = GfxDevice::CreateVertexBuffer(normals.size(), sizeof(Vec3f), normals.data(), name + " vertex norms buffer");
    if (!uv0.empty())  bufferHandle_uv0 = GfxDevice::CreateVertexBuffer(uv0.size(), sizeof(Vec2f), uv0.data(), name + " vertex uv0 buffer");
    if (!uvz0.empty())  bufferHandle_uvz0 = GfxDevice::CreateVertexBuffer(uvz0.size(), sizeof(Vec3f), uvz0.data(), name + " vertex uvz0 buffer");
    if (!uvz1.empty())  bufferHandle_uvz1 = GfxDevice::CreateVertexBuffer(uvz1.size(), sizeof(Vec3f), uvz1.data(), name + " vertex uvz1 buffer");
    if (!uvzw0.empty())  bufferHandle_uvzw0 = GfxDevice::CreateVertexBuffer(uvzw0.size(), sizeof(Vec4f), uvzw0.data(), name + " vertex uvzw0 buffer");
    if (!colors.empty())  bufferHandle_colors = GfxDevice::CreateVertexBuffer(colors.size(), sizeof(Vec4f), colors.data(), name + " vertex colors buffer");
    if (!indices.empty())  bufferHandle_indices = GfxDevice::CreateIndexBuffer(indices.size(), IndexFormat::UShort, indices.data(), name + " iBuffer");
}

Primitive Primitive::NewPlainTriangle()
{
    Primitive prim;
    prim.vertices = {
        Vec3f(-1.0f, -1.0f, 0.0f),
        Vec3f(1.f, -1.f, 0.0f),
        Vec3f(0.f, 1.f, 0.0f)
    };
    prim.colors = {
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
    };
    prim.indices = {0, 1, 2};

    prim.topologyType = TopologyType::TriangleList;
    prim.RecalcLocalBounds();
    prim.CreateGfxBuffers();
    return prim;
}

Primitive Primitive::NewPlainQuad()
{
    Primitive prim;
    prim.vertices = {
        Vec3f(-1.0f, -1.0f, 0.0f),
        Vec3f(1.f, -1.f, 0.0f),
        Vec3f(-1.f, 1.f, 0.0f),
        Vec3f(1.f, 1.f, 0.0f)
    };
    prim.uv0 = {
        Vec2f(0.0f, 1.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(0.0f, 0.0f),
        Vec2f(1.0f, 0.0f)
    };
    prim.colors = {
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
    };
    prim.indices = {0, 1, 2, 3};

    prim.topologyType = TopologyType::TriangleStrip;
    prim.RecalcLocalBounds();
    prim.CreateGfxBuffers();
    return prim;
}

Primitive Primitive::NewCube()
{
    Primitive prim;
	prim.vertices = {
		Vec3f(-1.0f, -1.0f,  1.0f), // Front bottom left
        Vec3f( 1.0f, -1.0f,  1.0f), // Front bottom right
        Vec3f(-1.0f,  1.0f,  1.0f), // Front top left
        Vec3f( 1.0f,  1.0f,  1.0f), // Front top right

       	Vec3f(-1.0f, -1.0f, -1.0f), // Back bottom left
        Vec3f( 1.0f, -1.0f, -1.0f), // Back bottom right
        Vec3f(-1.0f,  1.0f, -1.0f), // Back top left
        Vec3f( 1.0f,  1.0f, -1.0f) // Back top right
    };
	prim.normals = {
		Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f()
    };
	prim.uv0 = {
		Vec2f(), Vec2f(), Vec2f(), Vec2f(), Vec2f(), Vec2f(), Vec2f(), Vec2f(),
    };
	prim.colors = {
		Vec4f(1.0f, 0.0f, 0.0f, 1.0f),
        Vec4f(0.0f, 1.0f, 0.0f, 1.0f),
        Vec4f(1.0f, 0.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f),

       	Vec4f(1.0f, 0.0f, 0.0f, 1.0f),
        Vec4f(0.0f, 1.0f, 0.0f, 1.0f),
       	Vec4f(1.0f, 0.0f, 1.0f, 1.0f),
        Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
    };
	prim.indices = {
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	};
    prim.topologyType = TopologyType::TriangleStrip;
    prim.RecalcLocalBounds();
    prim.CreateGfxBuffers();
    return prim;
}