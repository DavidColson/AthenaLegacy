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
    texcoords = eastl::vector<Vec2f>(copy.texcoords);
    colors = eastl::vector<Vec4f>(copy.colors);
    indices = eastl::vector<uint16_t>(copy.indices);
    topologyType = copy.topologyType;

    CreateGfxBuffers();
}

Primitive::Primitive(Primitive&& copy)
{
    vertices = copy.vertices;
    normals = copy.normals;
    texcoords = copy.texcoords;
    colors = copy.colors;
    indices = copy.indices;
    topologyType = copy.topologyType;

    gfxVerticesBuffer = copy.gfxVerticesBuffer;
    gfxNormalsBuffer = copy.gfxNormalsBuffer;
    gfxTexcoordsBuffer = copy.gfxTexcoordsBuffer;
    gfxColorsBuffer = copy.gfxColorsBuffer;
    gfxIndexBuffer = copy.gfxIndexBuffer;

    copy.gfxVerticesBuffer = INVALID_HANDLE;
    copy.gfxNormalsBuffer = INVALID_HANDLE;
    copy.gfxTexcoordsBuffer = INVALID_HANDLE;
    copy.gfxColorsBuffer = INVALID_HANDLE;
    copy.gfxIndexBuffer = INVALID_HANDLE;
}

Primitive& Primitive::operator=(const Primitive& copy)
{
    GfxDevice::FreeVertexBuffer(gfxVerticesBuffer);
    GfxDevice::FreeVertexBuffer(gfxNormalsBuffer);
    GfxDevice::FreeVertexBuffer(gfxTexcoordsBuffer);
    GfxDevice::FreeVertexBuffer(gfxColorsBuffer);
    GfxDevice::FreeIndexBuffer(gfxIndexBuffer);

    vertices = eastl::vector<Vec3f>(copy.vertices);
    normals = eastl::vector<Vec3f>(copy.normals);
    texcoords = eastl::vector<Vec2f>(copy.texcoords);
    colors = eastl::vector<Vec4f>(copy.colors);
    indices = eastl::vector<uint16_t>(copy.indices);
    topologyType = copy.topologyType;
    CreateGfxBuffers();
    
    return *this;
}

Primitive& Primitive::operator=(Primitive&& copy)
{
    vertices = copy.vertices;
    normals = copy.normals;
    texcoords = copy.texcoords;
    colors = copy.colors;
    indices = copy.indices;
    topologyType = copy.topologyType;

    gfxVerticesBuffer = copy.gfxVerticesBuffer;
    gfxNormalsBuffer = copy.gfxNormalsBuffer;
    gfxTexcoordsBuffer = copy.gfxTexcoordsBuffer;
    gfxColorsBuffer = copy.gfxColorsBuffer;
    gfxIndexBuffer = copy.gfxIndexBuffer;

    copy.gfxVerticesBuffer = INVALID_HANDLE;
    copy.gfxNormalsBuffer = INVALID_HANDLE;
    copy.gfxTexcoordsBuffer = INVALID_HANDLE;
    copy.gfxColorsBuffer = INVALID_HANDLE;
    copy.gfxIndexBuffer = INVALID_HANDLE;

    return *this;
}

Primitive::~Primitive()
{
    GfxDevice::FreeVertexBuffer(gfxVerticesBuffer);
    GfxDevice::FreeVertexBuffer(gfxNormalsBuffer);
    GfxDevice::FreeVertexBuffer(gfxTexcoordsBuffer);
    GfxDevice::FreeVertexBuffer(gfxColorsBuffer);
    GfxDevice::FreeIndexBuffer(gfxIndexBuffer);
}

void Primitive::CreateGfxBuffers()
{
    if (!vertices.empty()) gfxVerticesBuffer = GfxDevice::CreateVertexBuffer(vertices.size(), sizeof(Vec3f), vertices.data(), name + " vertex position buffer");
    if (!normals.empty()) gfxNormalsBuffer = GfxDevice::CreateVertexBuffer(normals.size(), sizeof(Vec3f), normals.data(), name + " vertex norms buffer");
    if (!texcoords.empty())  gfxTexcoordsBuffer = GfxDevice::CreateVertexBuffer(texcoords.size(), sizeof(Vec2f), texcoords.data(), name + " vertex texcoords buffer");
    if (!colors.empty())  gfxColorsBuffer = GfxDevice::CreateVertexBuffer(colors.size(), sizeof(Vec4f), colors.data(), name + " vertex colors buffer");
    if (!indices.empty())  gfxIndexBuffer = GfxDevice::CreateIndexBuffer(indices.size(), IndexFormat::UShort, indices.data(), name + " iBuffer");
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
    prim.texcoords = {
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
	prim.texcoords = {
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
    prim.CreateGfxBuffers();
    return prim;
}