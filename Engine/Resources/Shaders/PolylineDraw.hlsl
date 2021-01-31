
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

#include "Engine/Resources/Shaders/Common.hlsl"

struct VertInput
{
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD0;
    float3 prev : TEXCOORD1;
    float3 next : TEXCOORD2;
    float4 color : COLOR;
    uint instanceId : SV_InstanceID;
};

struct VertOutput
{   
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
    float4 color: COLOR;
    float thicknessPixels: THICKPIXELS;
};

#define ANTI_ALIASING

VertOutput VSMain(VertInput vertIn)
{
	VertOutput output;

    float4x4 toClipTransform = worldToClipTransform;
    if (array[vertIn.instanceId].isScreenSpace == 1)
        toClipTransform = screenSpaceToClipTransform;

    // Scale breaks out careful transformations, so we must do it before we start
    float3 transformScale;
    float4x4 transformScaleless = RemoveScaling(array[vertIn.instanceId].transform, transformScale);
    vertIn.pos *= float4(transformScale, 1);
    vertIn.prev *= float4(transformScale, 1);
    vertIn.next *= float4(transformScale, 1);

    float thickness = vertIn.uv.z * 0.5;

    float4 tanPrev = float4(vertIn.prev, 1.0) - vertIn.pos;
    float4 tanNext = vertIn.pos - float4(vertIn.next, 1.0);

    float4 normPrev;
    float4 normNext;
    if (array[vertIn.instanceId].zAlign)
    {
        normPrev = normalize(float4(cross(tanPrev.xyz, -float3(0.0, 0.0, 1.0)), 0.0));
        normNext = normalize(float4(cross(tanNext.xyz, -float3(0.0, 0.0, 1.0)), 0.0));
    }
    else
    {
        float3 invDirectionToCam = vertIn.pos.xyz - camWorldPosition;
        normPrev = normalize(float4(cross(tanPrev.xyz, -invDirectionToCam), 0.0));
        normNext = normalize(float4(cross(tanNext.xyz, -invDirectionToCam), 0.0));
    }

    float4 cornerBisector = (normPrev + normNext) / 2.0;
    cornerBisector = cornerBisector / dot(cornerBisector, normNext);


    float pixelsPerMeter = 1;
    if (array[vertIn.instanceId].isScreenSpace == 0)
        pixelsPerMeter = WorldDistanceInPixels(vertIn.pos.xyz, vertIn.pos.xyz + normalize(cornerBisector).xyz);
    
    float thicknessPixelsDesired = thickness * pixelsPerMeter;
    #if defined(ANTI_ALIASING)
        float thicknessPixels = max(0.5, thicknessPixelsDesired + 1.0);
    #else
        float thicknessPixels = max(0.5, thicknessPixelsDesired);
    #endif
    thickness = thicknessPixels / pixelsPerMeter;
    float2 uvScale = float2(1.0, 1.0);
    uvScale.y = thicknessPixels / max(0.00001, thicknessPixelsDesired);

    float4 vertPos = vertIn.pos + cornerBisector * vertIn.uv.y * thickness;

    #if defined(ANTI_ALIASING)
        float endPointExtrude = 1.0 / pixelsPerMeter;
        vertPos -= tanNext * vertIn.uv.x * endPointExtrude;
    #endif

	output.pos = mul(vertPos, mul(transformScaleless, toClipTransform));
	output.color = vertIn.color;
	output.uv = vertIn.uv.xy * uvScale;
	output.thicknessPixels = thicknessPixels;

    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    float4 result = pixelIn.color;

#if defined(ANTI_ALIASING)
    float2 distanceToLine = 1.0 - abs(pixelIn.uv);
    float mask = 1.0;
    mask = min(mask, AntiAliasEdge(pixelIn.uv.y, distanceToLine.y, pixelIn.thicknessPixels));
    mask = min(mask, AntiAliasEdge(pixelIn.uv.x, distanceToLine.x, 0.0));
    result.a *= mask;
#endif

    clip(result.a - 0.0001);

    return result;
}