struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Col: COLOR;
	float2 Tex: TEXCOORD0;
};

cbuffer cbPostProcessShaderData
{
	float2 resolution;
	float time;
};

float2 curve(float2 uv)
{
	// Figure out what the shit is happening here and make it more understandable
	uv = (uv - 0.5) * 2.0;
	uv *= 1.1;	
	uv.x *= 1.0 + pow((abs(uv.y) / 5.0), 2.0);
	uv.y *= 1.0 + pow((abs(uv.x) / 4.0), 2.0);
	uv  = (uv / 2.0) + 0.5;
	uv =  uv *0.92 + 0.04;
	return uv;
}

VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR, float2 inTex : TEXCOORD0)
{
	VS_OUTPUT output;

	output.Pos = inPos;
	output.Col = float4(1, 1, 1, 1);
	output.Tex = inTex;

	return output;
}

Texture2D originalFrame : register(t0);
Texture2D blurredFrame : register(t1);
SamplerState SampleType;
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	// Comment out to disable post processing
	//return blurredFrame.Sample(SampleType, input.Tex);

	// Warp the UV coordiantes to make the screen warp effect
	float2 q = input.Tex.xy;
	float2 uv = q;
	uv = curve( uv );

	float3 col;


	// This offsets the UV lookuups over time and makes pixels move around a little bit
	float x =  
		  sin(0.3*time + uv.y*10.0)
		* sin(0.7*time + uv.y*30.0)
		* sin(0.3+0.33*time + uv.y*20.0)
		* 0.001;

	// Vignette
	float vig = (0.0 + 16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y));

	// Offsets for each colour channel lookup, use the vignette value to make the chromatic abberation worse at the edge of the screen
	float3 colorOffX = float3(0.0015, 0.0, -0.0017) * (1.0 - pow(vig, 0.8) + 0.3);
	float3 colorOffY = float3(0.0, -0.0016, 0.0) * (1.0 - pow(vig, 0.8) + 0.3);
	/*float3 colorOffX = float3(0.0, 0.0, 0.0);
	float3 colorOffY = float3(0.0, 0.0, 0.0);*/

	// Sample the frame texture, and then offset the channels to create some chromatic abberation
	col.r = originalFrame.Sample(SampleType, float2(x + uv.x + colorOffX.r, uv.y + colorOffY.r)).x + 0.1;
	col.g = originalFrame.Sample(SampleType, float2(x + uv.x + colorOffX.g, uv.y + colorOffY.g)).y + 0.1;
	col.b = originalFrame.Sample(SampleType, float2(x + uv.x + colorOffX.b, uv.y + colorOffY.b)).z + 0.1;

	col += blurredFrame.Sample(SampleType, uv) * 1.5f;

	// Some kind of pre vignette tonemapping
	col = clamp(col*0.6+0.4*col*col*1.0,0.0,1.0);

	// Apply the vignette darkening
	col *= pow(vig, 0.3);
	col *= 2.8;

	// Draw scanlines
	float scans = clamp( 0.35 + 0.35 * sin(3.5 * time + uv.y * resolution.y * 1.5), 0.0, 1.0);
	float s = pow(scans,1.7);
	col = col*(0.5 + 0.2*s) ;

	// Makes the screen flash subtley
	col *= 1.0+0.01*sin(110.0*time);

	// Set colour in corners to black
	if (uv.x < 0.0 || uv.x > 1.0)
		col *= 0.0;
	if (uv.y < 0.0 || uv.y > 1.0)
		col *= 0.0;
	
	// Seems to be normalizing output?
	col*=1.0-0.65*clamp((fmod(input.Tex.x, 2.0)-1.0)*2.0,0.0,1.0);
	
	return float4(col,1.0);
}