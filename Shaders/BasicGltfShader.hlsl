cbuffer cbTransform\
{
    float4x4 WVP;\
};
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

VS_OUTPUT VSMain(float4 inPos : SV_POSITION)
{
    VS_OUTPUT output;
    output.Pos = mul(inPos, WVP);
    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0);
    //return input.Col;
}