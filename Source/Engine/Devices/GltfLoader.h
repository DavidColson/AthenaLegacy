#pragma once

#include "Rendering/Mesh.h"

// Actually owns the data
struct Buffer
{
    // @Incomplete: delete the bytes when you are finished
    char* pBytes{ nullptr };
    size_t byteLength{ 0 };
};

// Does not actually own the data
struct BufferView
{
    // pointer to some place in a buffer
    char* pBuffer{ nullptr };
    size_t length{ 0 };

    enum Target
    {
        Array,
        ElementArray
    };
    Target target;    
};

struct Accessor
{
    // pointer to some place in a buffer view
    char* pBuffer{ nullptr };
    int count{ 0 };
    enum ComponentType
    {
        Byte,
        UByte,
        Short,
        UShort,
        UInt,
        Float
    };
    ComponentType componentType;

    enum Type
    {
        Scalar,
        Vec2,
        Vec3,
        Vec4,
        Mat2,
        Mat3,
        Mat4
    };
    Type type;
};

struct GltfScene
{
    eastl::vector<Mesh> meshes;
};

GltfScene LoadGltf(const char* filename);