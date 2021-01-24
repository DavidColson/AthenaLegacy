

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
		float4 color;
		float3 lineStart;
		float thickness;
		float3 lineEnd;
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
    float4 col: COLOR;
    float capLengthRatio : CAPLENGTHRATIO;
    float thicknessPixels : THICKPIXELS;
};

#define ROUND_CAPS
#define ANTI_ALIASING

VertOutput VSMain(VertInput vertIn)
{
    VertOutput output;

    // Implement ability to set thickness as a pixels unit

    // We divide by half since we extrude in both directions by thickness
    float thickness = array[vertIn.instanceId].thickness * 0.5;

    float4 lineStart = float4(array[vertIn.instanceId].lineStart, 1.0);
    float4 lineEnd = float4(array[vertIn.instanceId].lineEnd, 1.0);
    float4 thisVert = (vertIn.vertexId % 2 == 0) ? lineStart : lineEnd;

    float4 diff = lineEnd - lineStart;

    float4 norm;
    if (array[vertIn.instanceId].zAlign)
        norm = normalize(float4(cross(diff.xyz, float3(0.0, 0.0, 1.0)), 0.0));
    else
    {
        float3 invDirectionToCam = thisVert.xyz - camWorldPosition;
        norm = normalize(float4(cross(diff.xyz, -invDirectionToCam), 0.0));
    }
    // This will set the thickness such that it's never less than 1 pixel on the screen
    float pixelsPerMeter = 1;
    if (array[vertIn.instanceId].isScreenSpace == 0)
        pixelsPerMeter = WorldDistanceInPixels(thisVert.xyz, thisVert.xyz + norm.xyz);

    float thicknessPixelsDesired = thickness * pixelsPerMeter;
    #if defined(ANTI_ALIASING)
        float thicknessPixels = max(0.5, thicknessPixelsDesired + 1.0);
    #else
        float thicknessPixels = max(0.5, thicknessPixelsDesired);
    #endif

    thickness = thicknessPixels / pixelsPerMeter;
    float2 uvScale = float2(1.0, 1.0);
    uvScale.y = thicknessPixels / max(0.00001, thicknessPixelsDesired);
    uvScale.x = (length(diff) + 1.0 / pixelsPerMeter) / length(diff);

    thisVert -= vertIn.pos.y * norm * thickness; // Extrude width
    #if defined(ROUND_CAPS)
        thisVert += vertIn.pos.x * normalize(diff) * thickness; // Extrude ends
    #endif

    float4x4 modelToClipTransform;
    if (array[vertIn.instanceId].isScreenSpace == 1)
        modelToClipTransform = mul(array[vertIn.instanceId].transform, screenSpaceToClipTransform);
    else
        modelToClipTransform = mul(array[vertIn.instanceId].transform, worldToClipTransform);

    output.pos = mul(thisVert, modelToClipTransform);
    output.col = array[vertIn.instanceId].color;
    output.uv = vertIn.pos.xy * uvScale;
    output.capLengthRatio = 2.0 * thickness / length(diff);

    // If you want distance fade, then pass in the desired pixel thickness here instead
    output.thicknessPixels = thicknessPixels;

    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    float4 result = pixelIn.col;

#if defined(ANTI_ALIASING)
    // Blur line edges
    float mask = 1.0;

    float2 distanceToLine = 1.0 - abs(pixelIn.uv);
    mask = min(mask, AntiAliasEdge(pixelIn.uv.y, distanceToLine.y, pixelIn.thicknessPixels));

    #if defined(ROUND_CAPS)
        float2 uv = abs(pixelIn.uv);
        uv.x = (uv.x - 1) / pixelIn.capLengthRatio + 1;
        float distanceToCap = 1.0 - length(max(0, uv));
        mask = min(mask, AntiAliasEdge(pixelIn.uv.y, distanceToCap, pixelIn.thicknessPixels));
    #else
        mask = min(mask, AntiAliasEdge(pixelIn.uv.x, distanceToLine.x, pixelIn.thicknessPixels));
    #endif

    result.a *= mask;
#endif

    clip(result.a - 0.0001);

    return result;
}