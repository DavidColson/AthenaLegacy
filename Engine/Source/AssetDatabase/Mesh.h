#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "GraphicsDevice.h"
#include "AssetDatabase.h"

#include <EASTL/string.h>
#include <EASTL/vector.h>

struct Primitive
{
    Primitive() {}
    Primitive(const Primitive& copy);
	Primitive(Primitive&& copy);
	Primitive& operator=(const Primitive& copy);
	Primitive& operator=(Primitive&& copy);

    ~Primitive();

    eastl::string name{"Primitive"};

    eastl::vector<Vec3f> vertices{ nullptr };
    eastl::vector<Vec3f> normals{ nullptr };
    eastl::vector<Vec2f> texcoords{ nullptr };
    eastl::vector<Vec4f> colors{ nullptr };

    eastl::vector<uint16_t> indices{ nullptr };

    TopologyType topologyType{TopologyType::TriangleList};
    VertexBufferHandle gfxVerticesBuffer;
    VertexBufferHandle gfxNormalsBuffer;
    VertexBufferHandle gfxTexcoordsBuffer;
    VertexBufferHandle gfxColorsBuffer;
    IndexBufferHandle gfxIndexBuffer;

    void CreateGfxBuffers();

    // Primitive Mesh Creation
    static Primitive NewPlainQuad();
    static Primitive NewCube();
};

struct Mesh : public Asset
{
    virtual void Load(Path path, AssetHandle handleForThis) override;

    eastl::string name;
    eastl::vector<Primitive> primitives;

    void CreateGfxBuffers();
};

Primitive MakePlainQuad();