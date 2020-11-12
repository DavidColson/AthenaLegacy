#pragma once

#include "AssetDatabase.h"

#include "GraphicsDevice.h"

struct Shader : Asset
{
    Shader();
    virtual void Load(Path path, AssetHandle handleForThis) override;
    virtual void Reload(Path loadPath, AssetHandle handleForThis) override;
    ~Shader();

    PixelShaderHandle pixelShader;
    VertexShaderHandle vertShader;

    ProgramHandle program;
};
