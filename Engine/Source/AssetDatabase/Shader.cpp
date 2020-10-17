#include "Shader.h"

#include "Scanning.h"
#include "Log.h"
#include "FileSystem.h"


void Shader::Load(Path path)
{
    eastl::string contents = FileSys::ReadWholeFile(path);

    eastl::string typeString;

    Scan::ScanningState scanner;
    scanner.file = contents;
	scanner.current = 0;
	scanner.line = 1;
    if (Scan::Peek(scanner) == '#')
    {
        eastl::string_view typePragmaDecl = scanner.file.substr(scanner.current, 12);
        if (typePragmaDecl == "#pragma type")
        {
            scanner.current += 12;
            int start = scanner.current;
            while (Scan::Peek(scanner) != '\n' && Scan::Peek(scanner) != '\r')
                Scan::Advance(scanner);
            typeString = scanner.file.substr(start, scanner.current - start);
            typeString.ltrim();
            typeString.rtrim();
        }        
    }

    if (typeString == "scene3D")
    {
        eastl::vector<VertexInputElement> layout;
        layout.push_back({"SV_POSITION",AttributeType::Float3});
        layout.push_back({"NORMAL",AttributeType::Float3});
        layout.push_back({"TEXCOORD_",AttributeType::Float2});
        layout.push_back({"COLOR_",AttributeType::Float4});
        vertShader = GfxDevice::CreateVertexShader(contents, "VSMain", layout, path.AsRawString());
        pixelShader = GfxDevice::CreatePixelShader(contents, "PSMain", path.AsRawString());
        program = GfxDevice::CreateProgram(vertShader, pixelShader);
    }
    else if (typeString == "postprocess")
    {
        eastl::vector<VertexInputElement> layout;
        layout.push_back({"POSITION", AttributeType::Float3});
        layout.push_back({"TEXCOORD", AttributeType::Float2});
        vertShader = GfxDevice::CreateVertexShader(contents, "VSMain", layout, "Post processing");
        pixelShader = GfxDevice::CreatePixelShader(contents, "PSMain", "Post processing");
        program = GfxDevice::CreateProgram(vertShader, pixelShader);
    }
}

Shader::~Shader()
{
    GfxDevice::FreePixelShader(pixelShader);
    GfxDevice::FreeVertexShader(vertShader);
    GfxDevice::FreeProgram(program);
}