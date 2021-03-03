cbuffer PerSceneData : register(b0)
{
    float4x4 worldToClipTransform;
    float4x4 worldToCameraTransform;
    float4x4 screenSpaceToClipTransform;
    float3 camWorldPosition;
    float2 screenDimensions;
}

cbuffer InstanceData : register(b1)
{
    struct {
		float4x4 transform;
	    float4 color;
		float3 point1;
		float3 point2;
		float3 point3;
	} array[256];
}

// Included after per scene and object data as it references the above data
#include "Engine/Resources/Shaders/Common.hlsl"

struct VertInput
{
    float4 pos : SV_POSITION;
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
};

struct VertOutput
{   
    float4 pos : SV_POSITION;
    float4 color: COLOR1;
};


VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;

    float4 thisVert;
    switch( vertIn.vertexId )
    {
        case 0: thisVert = float4(array[vertIn.instanceId].point1, 1.0); break;
        case 1: thisVert = float4(array[vertIn.instanceId].point2, 1.0); break;
        case 2: thisVert = float4(array[vertIn.instanceId].point3, 1.0); break;
    }

    output.pos = mul(thisVert, mul(array[vertIn.instanceId].transform, worldToClipTransform));
    output.color = array[vertIn.instanceId].color;
    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    return pixelIn.color;
}