

cbuffer PerSceneData : register(b0)
{
    float4x4 worldToClipTransform;
    float4x4 worldToCameraTransform;
    float4x4 screenSpaceToClipTransform;
    float3 camWorldPosition;
    float2 screenDimensions;
}

// Included after per scene and object data as it references the above data
#include "Engine/Resources/Shaders/Common.hlsl"

cbuffer InstanceData : register(b1)
{
	struct {
		float4x4 transform;
		float3 location;
        float strokeSize;
        float4 strokeColor;
        float4 fillColor;
        float4 cornerRadius;
        float2 size;
        int isScreenSpace;
		int zAlign;
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
    float2 uvScale : UV_SCALE;
    float4 cornerRadius : CORNER_RAD;
    float strokeSize : BORDER_SIZE;
};

#define ANTI_ALIASING

VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;
    
    float3 scale;
    float4x4 transformScaleless = RemoveScaling(array[vertIn.instanceId].transform, scale);

    float2 uvScale = float2(array[vertIn.instanceId].size.x, array[vertIn.instanceId].size.y) * 0.5;

    float4x4 toClipTransform = worldToClipTransform;
    if (array[vertIn.instanceId].isScreenSpace == 1)
        toClipTransform = screenSpaceToClipTransform;

    float3 right;
    float3 up;
    if (array[vertIn.instanceId].zAlign)
    {
        right = float3(1, 0, 0);
        up = float3(0, 1, 0);
    }
    else
    {
        right = mul(float3(1, 0, 0), (float3x3)transpose(worldToCameraTransform));
        up = mul(float3(0, 1, 0), (float3x3)transpose(worldToCameraTransform));
    }

    float3 vertWorldPos = array[vertIn.instanceId].location + (right * vertIn.pos.x * uvScale.x * scale.x) + (up * vertIn.pos.y * uvScale.y * scale.y);

    output.pos = mul(float4(vertWorldPos, 1), mul(transformScaleless, toClipTransform));
    output.fillcol = array[vertIn.instanceId].fillColor;
    output.strokecol = array[vertIn.instanceId].strokeColor;
    output.uv = vertIn.pos * uvScale;
    output.uvScale = uvScale;
    output.cornerRadius = array[vertIn.instanceId].cornerRadius;
    output.strokeSize = array[vertIn.instanceId].strokeSize;

    return output;
}

float sdf( float2 queryPoint, float2 boxSize, float4 cornerRadius)
{
    // This figures out which quadrant we're in and puts that corner radius value into X
    cornerRadius.xy = (queryPoint.x>0.0)?cornerRadius.xy : cornerRadius.zw;
    cornerRadius.x  = (queryPoint.y>0.0)?cornerRadius.x  : cornerRadius.y;

    float2 dist = abs(queryPoint) - boxSize + cornerRadius.x;
    return length(max(dist, 0.0)) + min(max(dist.x, dist.y), 0.0) - cornerRadius.x;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    float4 result = pixelIn.fillcol;

    float dist = sdf(pixelIn.uv, pixelIn.uvScale, pixelIn.cornerRadius);
    
    // TODO: Need to boost thickness when screen space stroke width is smaller than size of blur

    #if defined(ANTI_ALIASING)
        float blurAmount = 2.0;
        float grad = blurAmount * FWidthFancy(dist);

        float aaMask = smoothstep(0.0, 1.0, abs(dist) / grad);
        float strokeFillBlend = smoothstep(0.0, 1.0, (dist + pixelIn.strokeSize) / grad);

        if (dist > 0.0)
            result = float4(0.0, 0.0, 0.0, 0.0);
        else
            result = lerp(pixelIn.fillcol, pixelIn.strokecol, strokeFillBlend);

        result.a *= aaMask;
    #else
        if (dist < -pixelIn.strokeSize)
            return pixelIn.fillcol;
        else if (dist < 0.0)
            return pixelIn.strokecol;
        else
            return float4(0.0, 0.0, 0.0, 0.0);
    #endif

    clip(result.a - 0.0001);

    return result;
}