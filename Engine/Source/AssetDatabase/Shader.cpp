#include "Shader.h"

#include "Scanning.h"
#include "Log.h"
#include "FileSystem.h"


void Shader::Load(Path path, AssetHandle handleForThis)
{
    eastl::string contents = FileSys::ReadWholeFile(path);

    vertShader = GfxDevice::CreateVertexShader(contents, "VSMain", path.AsRawString());
    pixelShader = GfxDevice::CreatePixelShader(contents, "PSMain", path.AsRawString());
    program = GfxDevice::CreateProgram(vertShader, pixelShader);
}

Shader::~Shader()
{
    GfxDevice::FreePixelShader(pixelShader);
    GfxDevice::FreeVertexShader(vertShader);
    GfxDevice::FreeProgram(program);
}