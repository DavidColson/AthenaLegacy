

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
        float radius;
        float4 fillColor;
        float4 strokeColor;
        float strokeWidth;
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
    float4 fillcol: COLOR1;
    float4 strokecol: COLOR2;
    float strokeWidth : STROKE_WIDTH;
    float radius : RADIUS;
};


VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;
    
    float radius = array[vertIn.instanceId].radius;

    float4x4 modelToWorld = {radius, 0.0, 0.0, 0.0,
                             0.0, radius, 0.0, 0.0,
                             0.0, 0.0, 1.0, 0.0,
                             array[vertIn.instanceId].location.x, array[vertIn.instanceId].location.y, array[vertIn.instanceId].location.z, 1.0};

    float4x4 modelToClipTransform = mul(modelToWorld, worldToClipTransform);

    output.pos = mul(vertIn.pos, modelToClipTransform);
    output.fillcol = array[vertIn.instanceId].fillColor;
    output.strokecol = array[vertIn.instanceId].strokeColor;
    output.uv = vertIn.pos.xy * radius;
    output.strokeWidth = array[vertIn.instanceId].strokeWidth;
    output.radius = array[vertIn.instanceId].radius;

    return output;
}

float sdf( float2 queryPoint, float radius)
{
    return length(queryPoint) - radius;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    float dist = sdf(pixelIn.uv, pixelIn.radius);

    if (dist < -pixelIn.strokeWidth)
        return pixelIn.fillcol;
    else if (dist < 0.0)
        return pixelIn.strokecol;
    else
        return float4(0.0, 0.0, 0.0, 0.0);
}