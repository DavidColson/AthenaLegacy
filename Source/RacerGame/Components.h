#pragma once

#include <TypeSystem.h>
#include <GraphicsDevice.h>

struct CRenderable
{
    VertexBufferHandle vBuffer;
    IndexBufferHandle iBuffer;
    ProgramHandle program;
    ConstBufferHandle constBuffer;
    int indexCount;
    TopologyType type{ TopologyType::TriangleStrip };
};

struct CCamera
{
    float fov{ 60.0f };
    float horizontalAngle{ 0.0f };
    float verticalAngle{ 0.0f };

    REFLECT()
};