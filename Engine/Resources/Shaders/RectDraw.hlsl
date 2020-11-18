

cbuffer PerSceneData : register(b0)
{
    float3 camWorldPosition;
    float2 screenDimensions;
}

cbuffer PerObjectData : register(b1)
{
    float4x4 worldToClipTransform;
};

// Included after per scene and object data as it references the above data
#include "Engine/Resources/Shaders/Common.hlsl"

cbuffer InstanceData : register(b2)
{
	struct {
		float3 location;
        float borderSize;
        float4 color;
        float4 cornerRadius;
        float2 size;
	} array[256];
};

struct VertInput
{
    float4 pos : SV_POSITION;
    uint instanceId : SV_InstanceID;
    uint vertexId : SV_VertexID;
};

struct VertOutput
{   
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
    float4 col: COLOR;
};

#define ANTI_ALIASING
//#define Z_ALIGN

VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;
    
    float4x4 modelToWorld = {1.0, 0.0, 0.0, 0.0,
                             0.0, 1.0, 0.0, 0.0,
                             0.0, 0.0, 1.0, 0.0,
                             array[vertIn.instanceId].location.x, array[vertIn.instanceId].location.y, array[vertIn.instanceId].location.z, 1.0};

    float4x4 modelToClipTransform = mul(modelToWorld, worldToClipTransform);

    output.pos = mul(vertIn.pos, modelToClipTransform);
    output.col = array[vertIn.instanceId].color;
    output.uv = vertIn.pos;

    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    float4 result = pixelIn.col;

    return result;
}