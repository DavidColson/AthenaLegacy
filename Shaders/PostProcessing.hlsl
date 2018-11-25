struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Col: COLOR;
	float2 Tex: TEXCOORD0;
};

VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR, float2 inTex : TEXCOORD0)
{
	VS_OUTPUT output;

	output.Pos = inPos;
	output.Col = float4(1, 1, 1, 1);
	output.Tex = inTex;

	return output;
}

Texture2D shaderTexture;
SamplerState SampleType;
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	float4 textureColor;
	textureColor = shaderTexture.Sample(SampleType, input.Tex);
	//textureColor = float4(1.0, 0.0, 0.0, 1.0);
	return textureColor;
}