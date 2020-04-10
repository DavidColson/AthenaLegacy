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