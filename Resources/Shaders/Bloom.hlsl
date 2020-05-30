struct VS_OUTPUT
{
  float4 Pos : SV_POSITION;
  float2 Tex: TEXCOORD0;
};

cbuffer cbBloomShaderData
{
	float2 direction;
	float2 resolution;
};

VS_OUTPUT VSMain(float4 inPos : POSITION, float2 inTex : TEXCOORD0)
{
  VS_OUTPUT output;

  output.Pos = inPos;
  output.Tex = inTex;

  return output;
}

float CalcGauss(float x, float sigma)
{
	if (sigma <= 0.0)
		return 0.0;
	return exp(-(x*x) / (2.0 * sigma)) / (2.0 * 3.14157 * sigma);
}

Texture2D shaderTexture;
SamplerState SampleType;
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	float sigma = 0.8;

	float4 texCol = shaderTexture.Sample(SampleType, input.Tex);
	float4 gaussCol = float4(texCol.rgb, 1.0);
	float2 step = direction / resolution;

	// blur in step direction, adding the total weight in the alpha channel
	for (int i = 1; i <= 32; ++i)
	{
		float weight = CalcGauss(float(i) / 32.0, sigma * 0.5);
		if (weight < 1.0 / 255.0)
			break;
		texCol = shaderTexture.Sample(SampleType, input.Tex + step * float(i));
		gaussCol += float4(texCol.rgb * weight, weight);
		texCol = shaderTexture.Sample(SampleType, input.Tex - step * float(i));
		gaussCol += float4(texCol.rgb * weight, weight);
	}
	// Normalizing the output
	gaussCol.rgb = clamp(gaussCol.rgb / gaussCol.w, 0.0, 1.0);

  return float4(gaussCol.rgb,1.0);
}