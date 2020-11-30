

cbuffer PerSceneData : register(b0)
{
    float4x4 worldToClipTransform;
    float3 camWorldPosition;
    float2 screenDimensions;
}

cbuffer InstanceData : register(b1)
{
    float4x4 transform;
}

// Included after per scene and object data as it references the above data
#include "Engine/Resources/Shaders/Common.hlsl"

struct VertInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR1;
};

struct VertOutput
{   
    float4 pos : SV_POSITION;
    float4 color: COLOR1;
};


VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;
    
    output.pos = mul(vertIn.pos, mul(transform, worldToClipTransform));
    output.color = vertIn.col;
    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    return pixelIn.color;
}