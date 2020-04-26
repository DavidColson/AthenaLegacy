#include "GltfLoader.h"

#include "Json.h"
#include "Log.h"
#include "Base64.h"
#include "Vec3.h"
#include "GraphicsDevice.h"
#include "Rendering/Mesh.h" 

#include <SDL.h>

char* ReadFile(const char* filename)
{
	SDL_RWops* rw = SDL_RWFromFile(filename, "r+");
	if (rw == nullptr)
	{
		Log::Warn("%s failed to load", filename);
		return nullptr;
	}

	size_t fileSize = SDL_RWsize(rw);

	char* buffer = new char[fileSize];
	SDL_RWread(rw, buffer, sizeof(char) * fileSize, 1);
	SDL_RWclose(rw);
	buffer[fileSize] = '\0';
	return buffer;
}

GltfScene LoadGltf(const char* filename)
{
    // Load file
    eastl::string file = ReadFile(filename);

    // Parse json
    JsonValue parsed = ParseJsonFile(file);

    // Check for asset label
    bool validGltf = parsed["asset"]["version"].ToString() == "2.0";
    if (!validGltf)
    {
        Log::Crit("Failed to load Gltf file, invalid contents: %s", filename);
    }

    GltfScene scene;
    for (int i = 0; i < parsed["buffers"].Count(); i++)
    {
        Buffer buf;
        buf.byteLength = parsed["buffers"][i]["byteLength"].ToInt();
        buf.pBytes = new char[buf.byteLength];
        
        eastl::string encodedBuffer = parsed["buffers"][i]["uri"].ToString().substr(37);
        memcpy(buf.pBytes, DecodeBase64(encodedBuffer).data(), buf.byteLength);

        scene.rawDataBuffers.push_back(buf);
    }

    eastl::vector<BufferView> bufferViews;
    for (int i = 0; i < parsed["bufferViews"].Count(); i++)
    {
        BufferView view;

        int bufIndex = parsed["bufferViews"][i]["buffer"].ToInt();
        view.pBuffer = scene.rawDataBuffers[bufIndex].pBytes + parsed["bufferViews"][i]["byteOffset"].ToInt(); //@Incomplete, byte offset could not be provided, in which case we assume 0

        view.length = parsed["bufferViews"][i]["byteLength"].ToInt();

        // @Incomplete, target may not be provided
        int target = parsed["bufferViews"][i]["target"].ToInt();
        if (target == 34963)
            view.target = BufferView::ElementArray;
        else if (target = 34962)
            view.target = BufferView::Array;
        bufferViews.push_back(view);
    }

    eastl::vector<Accessor> accessors;
    for (int i = 0; i < parsed["accessors"].Count(); i++)
    {
        Accessor acc;

        int idx = parsed["accessors"][i]["bufferView"].ToInt();
        acc.pBuffer = bufferViews[idx].pBuffer + parsed["accessors"][i]["byteOffset"].ToInt();
        
        acc.count = parsed["accessors"][i]["count"].ToInt();

        int compType = parsed["accessors"][i]["componentType"].ToInt();
        switch (compType)
        {
        case 5120: acc.componentType = Accessor::Byte; break;
        case 5121: acc.componentType = Accessor::UByte; break;
        case 5122: acc.componentType = Accessor::Short; break;
        case 5123: acc.componentType = Accessor::UShort; break;
        case 5125: acc.componentType = Accessor::UInt; break;
        case 5126: acc.componentType = Accessor::Float; break;
        default: break;
        }

        eastl::string type = parsed["accessors"][i]["type"].ToString();
        if (type == "SCALAR") acc.type = Accessor::Scalar;
        else if (type == "VEC2") acc.type = Accessor::Vec2;
        else if (type == "VEC3") acc.type = Accessor::Vec3;
        else if (type == "VEC4") acc.type = Accessor::Vec4;
        else if (type == "MAT2") acc.type = Accessor::Mat2;
        else if (type == "MAT3") acc.type = Accessor::Mat3;
        else if (type == "MAT4") acc.type = Accessor::Mat4;

        accessors.push_back(acc);
    }
    
    eastl::vector<Mesh>& meshes = scene.meshes;
    for (int i = 0; i < parsed["meshes"].Count(); i++)
    {
        JsonValue& jsonMesh = parsed["meshes"][i];
        Mesh mesh;
        mesh.name = jsonMesh.HasKey("name") ? jsonMesh["name"].ToString() : "";

        for (int j = 0; j < jsonMesh["primitives"].Count(); j++)
        {
            JsonValue& jsonPrimitive = jsonMesh["primitives"][i];
            Primitive prim;

            if (jsonPrimitive.HasKey("mode"))
            {
                switch (jsonPrimitive["mode"].ToInt())
                {
                case 0: prim.topologyType = TopologyType::PointList; break;
                case 1: prim.topologyType = TopologyType::LineList; break;
                case 3: prim.topologyType = TopologyType::LineStrip; break;
                case 4: prim.topologyType = TopologyType::TriangleList; break;
                case 5: prim.topologyType = TopologyType::TriangleStrip; break;
                default: Log::Crit("Unsupported topology type given in file %s", filename);
                    break;
                }
            }

            //@incomplete Do interlacing data here
            prim.pVertBuffer = (VertPos*)accessors[jsonPrimitive["attributes"]["POSITION"].ToInt()].pBuffer;
            prim.nVerts = accessors[jsonPrimitive["attributes"]["POSITION"].ToInt()].count;

            prim.pIndexBuffer = (unsigned short*)accessors[jsonPrimitive["indices"].ToInt()].pBuffer;
            prim.nIndices = accessors[jsonPrimitive["indices"].ToInt()].count;
            mesh.primitives.push_back(prim);
        }
        meshes.push_back(mesh);
    }

    return scene;
}