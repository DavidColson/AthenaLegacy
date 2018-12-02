cbuffer cbPerObject
{
	float4x4 WVP;
	float lineThickness;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Col: COLOR;
	float2 Tex: TEXCOORD0;
};

VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR, float2 inTex : TEXCOORD0)
{
	VS_OUTPUT output;

	output.Pos = mul(inPos, WVP);
	output.Col = float4(1, 1, 1, 1);
	output.Tex = inTex;

    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return input.Col;
}

[maxvertexcount(4)]
void GSMain(lineadj VS_OUTPUT input[4], inout TriangleStream<VS_OUTPUT> OutputStream)
{
	// edge 1 normal
	float4 edge1 = input[0].Pos - input[1].Pos;
	float3 edge1Norm = normalize(float3( -edge1.y, edge1.x , 0.0));

	// edge prime normal
	float4 edgePrime = input[1].Pos - input[2].Pos;
	float3 edgePrimeNorm = normalize(float3(-edgePrime.y, edgePrime.x, 0.0));

	// edge 2 normal
	float4 edge2 = input[2].Pos - input[3].Pos;
	float3 edge2Norm = normalize(float3(-edge2.y, edge2.x, 0.0));

	float3 corner1Bisector = normalize(float3((edge1Norm.x + edgePrimeNorm.x) / 2, (edge1Norm.y + edgePrimeNorm.y) / 2, 0.0));
	corner1Bisector = corner1Bisector / dot(corner1Bisector, edgePrimeNorm);
	float3 corner2Bisector = normalize(float3((edgePrimeNorm.x + edge2Norm.x) / 2, (edgePrimeNorm.y + edge2Norm.y) / 2, 0.0));
	corner2Bisector = corner2Bisector / dot(corner2Bisector, edgePrimeNorm);

	float offset = lineThickness * 0.001;

	float3 verts[4];
	verts[0] = input[1].Pos.xyz + corner1Bisector * offset;
	verts[1] = input[1].Pos.xyz - corner1Bisector * offset;
	verts[2] = input[2].Pos.xyz + corner2Bisector * offset;
	verts[3] = input[2].Pos.xyz - corner2Bisector * offset;


	VS_OUTPUT outputVert;
	for (int i = 0; i < 4; i++)
	{
		outputVert.Pos = float4(verts[i], 1.0);
		outputVert.Col = float4(1, 0, 0, 1);
		outputVert.Tex = input[1].Tex;

		OutputStream.Append(outputVert);
	}
}