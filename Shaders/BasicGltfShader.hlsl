cbuffer cbTransform\
{
    float4x4 WVP;\
};
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 worldPos : POSITION;
    float4 col : COLOR_0;
};

VS_OUTPUT VSMain(float4 inPos : SV_POSITION, float3 norm : NORMAL, float2 tex : TEXCOORD_0, float4 col : COLOR_0)
{
    VS_OUTPUT output;
    output.Pos = mul(inPos, WVP);
    output.worldPos = inPos;
    output.col = col;
    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    //return float4(1.0, 0.0, 0.0, 1.0);
    return input.col * 3.0;
}