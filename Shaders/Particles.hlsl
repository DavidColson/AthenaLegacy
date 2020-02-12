cbuffer cbPerObject
{
	float4x4 VP;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Col: COLOR;
	float2 Tex: TEXCOORD;
};

VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR, float2 inTex : TEXCOORD, uint instanceId : SV_InstanceID, column_major float4x4 instanceTrans : INSTANCE_TRANSFORM)
{
	VS_OUTPUT output;

    float4x4 wvp = mul(instanceTrans, VP);
	output.Pos = mul(inPos, wvp);
	output.Col = float4(1, 1, 1, 1);
	output.Tex = inTex;

    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return input.Col;
}