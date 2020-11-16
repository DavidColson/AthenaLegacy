

// Anti Aliasing functions
// ------------------------

float FWidthFancy(float value)
{
    float2 pd = float2(ddx(value), ddy(value));
	return sqrt( dot( pd, pd ) );
}

float AntiAliasEdge(float uvCoord, float distance, float pixelOffset, float blurAmount = 2.0)
{
    float gradientSize = blurAmount * FWidthFancy(uvCoord);
    return smoothstep(0.0, 1.0, distance / max(0.00001, gradientSize) + saturate(pixelOffset) * 0.5);
}

// Maths stuff
// ------------------------------

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
