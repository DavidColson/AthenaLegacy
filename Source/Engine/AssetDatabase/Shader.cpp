#include "Shader.h"

#include "File.h"

void Shader::Load(eastl::string path)
{
    eastl::string contents = File::ReadWholeFile(path);

    // @Improvement: define different shader types for different rendering passes
    // Each type has it's own component shader types, and it's own custom input layout
    eastl::vector<VertexInputElement> layout;
	layout.push_back({"SV_POSITION",AttributeType::Float3});
	layout.push_back({"NORMAL",AttributeType::Float3});
	layout.push_back({"TEXCOORD_",AttributeType::Float2});
	layout.push_back({"COLOR_",AttributeType::Float4});

	vertShader = GfxDevice::CreateVertexShader(contents, "VSMain", layout, path.c_str());
	pixelShader = GfxDevice::CreatePixelShader(contents, "PSMain", path.c_str());

	program = GfxDevice::CreateProgram(vertShader, pixelShader);

    AssetDB::RegisterAsset(this, path);
}

Shader::~Shader()
{
    GfxDevice::FreePixelShader(pixelShader);
    GfxDevice::FreeVertexShader(vertShader);
    GfxDevice::FreeProgram(program);
}