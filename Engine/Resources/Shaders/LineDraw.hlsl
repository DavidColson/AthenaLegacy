
cbuffer PerSceneData : register(b0)
{
    float3 camWorldPosition;
    float2 screenDimensions;
}

cbuffer PerObjectData : register(b1)
{
    float4x4 worldToClipTransform;
};

cbuffer InstanceData : register(b2)
{
	struct {
		float3 lineStart;
		float3 lineEnd;
		float thickness;
	} array[16];
};


struct VertInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    uint instanceId : SV_InstanceID;
    uint vertexId : SV_VertexID;
};

struct VertOutput
{   
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
    float4 col: COLOR;
    float capLengthRatio : CAPLENGTHRATIO;
};

float2 WorldToScreenSpaceNormalized(float3 pos)
{
    float4 clipSpace = mul(float4(pos, 1.0), worldToClipTransform);
    return clipSpace.xy / clipSpace.w;
}

float WorldDistanceInPixels(float3 pos1, float3 pos2)
{
    float2 screenSpace1 = WorldToScreenSpaceNormalized(pos1);
    float2 screenSpace2 = WorldToScreenSpaceNormalized(pos2);
    float2 distance = (screenSpace1 - screenSpace2) * screenDimensions.xy;
    return length(distance) * 0.5;
}

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

    float3 invDirectionToCam = thisVert.xyz - camWorldPosition;
    float4 diff = lineEnd - lineStart;
    // if you want z aligned just use Z world axis instead of invDirectionToCam
    float4 norm = normalize(float4(cross(diff, float4(-invDirectionToCam, 1.0)), 0.0));

    // This will set the thickness such that it's never less than 1 pixel on the screen
    float pixelsPerMeter = WorldDistanceInPixels(thisVert, thisVert + norm);
    float thicknessPixels = thickness * pixelsPerMeter;
    #if defined(ANTI_ALIASING)
        thicknessPixels = max(0.5, thicknessPixels + 1.0);
    #else
        thicknessPixels = max(0.5, thicknessPixels);
    #endif
    thickness = thicknessPixels / pixelsPerMeter;


    thisVert -= vertIn.pos.y * norm * thickness; // Extrude width
    #if defined(ROUND_CAPS)
        thisVert += vertIn.pos.x * normalize(diff) * thickness; // Extrude ends
    #endif

    output.pos = mul(thisVert, worldToClipTransform);
    output.col = vertIn.col;
    output.uv = vertIn.pos.xy;
    output.capLengthRatio = 2.0 * thickness / length(diff);

    return output;
}

float FWidthFancy(float value)
{
    float2 pd = float2(ddx(value), ddy(value));
	return sqrt( dot( pd, pd ) );
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    float4 result = pixelIn.col;

#if defined(ANTI_ALIASING)
    float aliasingTune = 2.0;
    float2 distanceToLine = 1.0 - abs(pixelIn.uv);
    
    // Blur line edges
    float mask = 1.0;
    float gradientSize = aliasingTune * FWidthFancy(pixelIn.uv.y);
    mask = min(mask, smoothstep(0.0, 1.0, distanceToLine.y / gradientSize));

    #if defined(ROUND_CAPS)
        float2 uv = abs(pixelIn.uv);
        uv.x = (uv.x - 1) / pixelIn.capLengthRatio + 1;
        float distanceToCap = 1.0 - length(max(0, uv));
        mask = min(mask, smoothstep(0.0, 1.0, distanceToCap / gradientSize));
    #else
        float gradSize = aliasingTune * FWidthFancy(pixelIn.uv.x);
        mask = min(mask, smoothstep(0.0, 1.0, distanceToLine.x / gradSize));
    #endif

    result.a *= mask;
#endif

    // Can do distance blend by multipling mask by clamped (how wide is this line in pixels)
    return result;
}