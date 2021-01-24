

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
        float radius;
        float4 color;
        float thickness;
        float angleStart;
        float angleEnd;
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
    float4 color: COLOR1;
    float radius : RADIUS;
    float thickness : THICKNESS;
    float angStart : ANGSTART;
    float angEnd : ANGEND;
};

#define ANTI_ALIASING

VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;
    
    float radius = array[vertIn.instanceId].radius;

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

    float3 vertWorldPos = array[vertIn.instanceId].location + right * vertIn.pos.x * radius + up * vertIn.pos.y * radius;

    output.pos = mul(float4(vertWorldPos, 1), mul(array[vertIn.instanceId].transform, toClipTransform));
    output.color = array[vertIn.instanceId].color;
    output.uv = vertIn.pos.xy * radius;
    output.radius = array[vertIn.instanceId].radius;
    output.thickness = array[vertIn.instanceId].thickness;
    output.angStart = array[vertIn.instanceId].angleStart;
    output.angEnd = array[vertIn.instanceId].angleEnd;

    return output;
}

float sdf( float2 queryPoint, float radius)
{
    return length(queryPoint) - radius;
}

float2 rotate( float2 v, float ang ){
	float2 a = float2( cos(ang), sin(ang) );
	return float2( a.x * v.x - a.y * v.y, a.y * v.x + a.x * v.y );
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{


    float dist = sdf(pixelIn.uv, pixelIn.radius);

    float segmentMask = 1.0;
    if (pixelIn.angStart != pixelIn.angEnd)
    {
        float angOffset = -(pixelIn.angEnd + pixelIn.angStart) * 0.5;
        float2 segmentMaskCoords = rotate(pixelIn.uv, angOffset);
        float ang = atan2(segmentMaskCoords.y, segmentMaskCoords.x);
        float sectorSize = abs(pixelIn.angEnd - pixelIn.angStart);

        #if defined(ANTI_ALIASING)
            float gradientSize = 2.0 * FWidthFancy(ang);
            if (gradientSize > 6.0) // Kinda hacky fix for the fact that the gradient across the 0/2PI angle is PI, not 0 
                gradientSize = 0.0;
            segmentMask = 1.0 - smoothstep(0.0, 1.0, (abs(ang) - sectorSize * 0.5) /  max(0.00001, gradientSize) + 0.0);
        #else
            segmentMask = 1.0 - step(0.0, (abs(ang) - sectorSize * 0.5));
        #endif
    }

    float thickness = pixelIn.radius;
    if (pixelIn.thickness != 0.0)
        thickness = pixelIn.thickness;

    #if defined(ANTI_ALIASING)
        float blurAmount = 2.0;
        float grad = blurAmount * FWidthFancy(dist);

        float outerRingMask = smoothstep(0.0, 1.0, abs(dist) / grad);
        float innerRingMask = 1.0;
        if (pixelIn.thickness != 0.0)
            innerRingMask = smoothstep(0.0, 1.0, abs(dist + thickness) / grad);
    #endif

    // TODO: Need to boost thickness when screen space stroke width is smaller than size of blur

    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    if (dist + thickness < 0.0)
        result = float4(0.0, 0.0, 0.0, 0.0);
    else if (dist < 0.0)
        result = pixelIn.color * segmentMask;

    #if defined(ANTI_ALIASING)
        result.a *= outerRingMask * innerRingMask;
    #endif

    clip(result.a - 0.0001);

    return result;
}