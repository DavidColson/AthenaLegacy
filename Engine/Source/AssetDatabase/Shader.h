#pragma once

#include "AssetDatabase.h"

#include "GraphicsDevice.h"

struct Shader : Asset
{
    virtual void Load(Path path, AssetHandle handleForThis) override;
    ~Shader();

    PixelShaderHandle pixelShader;
    VertexShaderHandle vertShader;

    ProgramHandle program;
};
