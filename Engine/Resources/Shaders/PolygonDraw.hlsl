

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
        int isScreenSpace;
	    int zAlign;
	} array[256];
}

// Included after per scene and object data as it references the above data
#include "Engine/Resources/Shaders/Common.hlsl"

struct VertInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR1;
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

    float4x4 toClipTransform = worldToClipTransform;
    if (array[vertIn.instanceId].isScreenSpace == 1)
        toClipTransform = screenSpaceToClipTransform;

    output.pos = mul(vertIn.pos, mul(array[vertIn.instanceId].transform, toClipTransform));
    output.color = vertIn.col;
    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    return pixelIn.color;
}