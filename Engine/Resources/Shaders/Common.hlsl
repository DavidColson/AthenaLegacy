

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

float4x4 RemoveScaling(in float4x4 m, out float3 scale)
{
    float sx = length(float3(m[0][0], m[0][1], m[0][2]));
    float sy = length(float3(m[1][0], m[1][1], m[1][2]));
    float sz = length(float3(m[2][0], m[2][1], m[2][2]));

    float invSX = 1.0 / sx;
    m[0][0] *= invSX;
    m[0][1] *= invSX;
    m[0][2] *= invSX;

    float invSY = 1.0 / sy;
    m[1][0] *= invSY;
    m[1][1] *= invSY;
    m[1][2] *= invSY;

    float invSZ = 1.0 / sz;
    m[2][0] *= invSZ;
    m[2][1] *= invSZ;
    m[2][2] *= invSZ;

    scale.x = sx;
    scale.y = sy;
    scale.z = sz;

    return m;
}
