#include "Shader.h"

#include "Scanning.h"
#include "Log.h"
#include "FileSystem.h"
#include "GraphicsDevice.h"

Shader::Shader()
{
    bOverrideReload = true;
}

void Shader::Load(Path path, AssetHandle handleForThis)
{
    eastl::string contents = FileSys::ReadWholeFile(path);

    vertShader = GfxDevice::CreateVertexShader(contents, "VSMain", path.AsRawString());
    pixelShader = GfxDevice::CreatePixelShader(contents, "PSMain", path.AsRawString());
    program = GfxDevice::CreateProgram(vertShader, pixelShader);
}

void Shader::Reload(Path loadPath, AssetHandle handleForThis)
{
    eastl::string contents = FileSys::ReadWholeFile(loadPath);

    VertexShaderHandle newVert = GfxDevice::CreateVertexShader(contents, "VSMain", loadPath.AsRawString());
    PixelShaderHandle newPixel = GfxDevice::CreatePixelShader(contents, "PSMain", loadPath.AsRawString());
    if (GfxDevice::IsValid(newVert) && GfxDevice::IsValid(newPixel))
    {
        GfxDevice::FreeProgram(program);
        program = GfxDevice::CreateProgram(newVert, newPixel);
        vertShader = newVert;
        pixelShader = newPixel;
    }
}

Shader::~Shader()
{
    GfxDevice::FreePixelShader(pixelShader);
    GfxDevice::FreeVertexShader(vertShader);
    GfxDevice::FreeProgram(program);
}