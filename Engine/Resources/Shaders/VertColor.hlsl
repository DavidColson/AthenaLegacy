// Constant buffer for transform info
cbuffer cbTransform
{
    float4x4 WVP;
};

// Constant buffer for shader properties (i.e. line thickness) 


// struct for vert input data
// Struct for vertex output data

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 col : COLOR_0;
};

VS_OUTPUT VSMain(float4 inPos : SV_POSITION, float3 norm : NORMAL, float2 tex : TEXCOORD_0, float4 col : COLOR_0)
{
    VS_OUTPUT output;
    output.Pos = mul(inPos, WVP);
    output.col = col;
    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return input.col;
}