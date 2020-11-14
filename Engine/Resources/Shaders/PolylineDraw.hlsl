
cbuffer PerSceneData : register(b0)
{
    float3 camWorldPosition;
    float2 screenDimensions;
}

cbuffer PerObjectData : register(b1)
{
    float4x4 worldToClipTransform;
};

struct VertInput
{
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD0;
    float3 prev : TEXCOORD1;
    float3 next : TEXCOORD2;
    // float4 color : COLOR;
    uint instanceId : SV_InstanceID;
};

struct VertOutput
{   
    float2 uv : TEXCOORD;
    float4 pos : SV_POSITION;
    float4 color: COLOR;
};

VertOutput VSMain(VertInput vertIn)
{
	VertOutput output;

    float3 invDirectionToCam = vertIn.pos.xyz - camWorldPosition;
    
    float4 tanPrev = float4(vertIn.prev, 1.0) - vertIn.pos;
    float4 tanNext = vertIn.pos - float4(vertIn.next, 1.0);;

    float4 normPrev = normalize(float4(cross(tanPrev, float4(-invDirectionToCam, 1.0)), 0.0));
    float4 normNext = normalize(float4(cross(tanNext, float4(-invDirectionToCam, 1.0)), 0.0));

    float4 normAverage = (normPrev + normNext) / 2.0;

    float4 vertPos = vertIn.pos + normAverage * vertIn.uv.y * 0.1;
	output.pos = mul(vertPos, worldToClipTransform);
	output.color = float4(1.0, 1.0, 1.0, 1.0);
	output.uv = vertIn.uv.xy;

    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    return pixelIn.color;
}