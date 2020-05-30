#pragma once

#include "AssetDatabase.h"

#include "GraphicsDevice.h"

struct Shader : Asset
{
    virtual void Load(eastl::string path) override;
    ~Shader();

    PixelShaderHandle pixelShader;
    VertexShaderHandle vertShader;

    ProgramHandle program;
};
