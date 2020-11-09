cbuffer cbPerObject
{
	float4x4 VP;
};

cbuffer InstanceData
{
	struct {
		column_major float4x4 transform;
	} array[64];
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex: TEXCOORD0;
	float4 Col: COLOR0;
};

VS_OUTPUT VSMain(float4 inPos : POSITION, float2 inTex : TEXCOORD0, float4 inCol : COLOR0, uint instanceId : SV_InstanceID)
{
	VS_OUTPUT output;

    float4x4 wvp = mul(array[instanceId].transform, VP);
	output.Pos = mul(inPos, wvp);
	output.Col = float4(1, 1, 1, 1);
	output.Tex = inTex;

    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return input.Col;
}