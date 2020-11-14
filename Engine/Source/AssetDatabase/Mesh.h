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

    eastl::vector<Vec3f> vertices;
    eastl::vector<Vec3f> normals;
    eastl::vector<Vec2f> uv0;
    eastl::vector<Vec3f> uvz0;
    eastl::vector<Vec3f> uvz1;
    eastl::vector<Vec4f> uvzw0;
    eastl::vector<Vec4f> colors;

    eastl::vector<uint16_t> indices{ nullptr };

    TopologyType topologyType{TopologyType::TriangleList};
    VertexBufferHandle bufferHandle_vertices;
    VertexBufferHandle bufferHandle_normals;
    VertexBufferHandle bufferHandle_uv0;
    VertexBufferHandle bufferHandle_uvz0;
    VertexBufferHandle bufferHandle_uvz1;
    VertexBufferHandle bufferHandle_uvzw0;
    VertexBufferHandle bufferHandle_colors;
    IndexBufferHandle bufferHandle_indices;

    void CreateGfxBuffers();

    // Primitive Mesh Creation
    // TODO: These should really return Mesh's not primitives.
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