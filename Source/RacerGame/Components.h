#pragma once

#include <TypeSystem.h>
#include <GraphicsDevice.h>

struct CCube
{
    VertexBufferHandle vBuffer;
    IndexBufferHandle iBuffer;
    ProgramHandle program;
    ConstBufferHandle constBuffer;
};

struct CCamera
{
    float fov{ 60.0f };
    float horizontalAngle{ 0.0f };
    float verticalAngle{ 0.0f };
};