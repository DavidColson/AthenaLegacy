
cbuffer PerSceneData : register(b0)
{
    float3 camWorldPosition;
}

cbuffer PerObjectData : register(b1)
{
    float4x4 worldToClipTransform;
};

cbuffer InstanceData : register(b2)
{
	struct {
		float3 lineStart;
		float3 lineEnd;
		float thickness;
	} array[16];
};


struct VertInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    uint instanceId : SV_InstanceID;
    uint vertexId : SV_VertexID;
};

struct VertOutput
{
    float4 pos : SV_POSITION;
    float4 col: COLOR;
};

VertOutput VSMain(VertInput vertIn)
{
    // When you get vertex start point from instance data, just set vertex to that, and then do world to clip.
    VertOutput output;

    // If edge to edge distance is less than 1 pixel, then clamp it. But how the fuck do we work that out? 
    // take distance between vertex and vertex + normal and convert to screen space by going to clip space then perspective divide and multipling by screen size.
    // This gives you 1 unit in world space in pixels. You have effectively pixels per meter.
    // Multiply thickness by pixels per meter, then clamp to 1, then divide by pixels per meter to get back to thickness in meters
    float thickness = array[vertIn.instanceId].thickness;

    float4 lineStart = float4(array[vertIn.instanceId].lineStart, 1.0);
    float4 lineEnd = float4(array[vertIn.instanceId].lineEnd, 1.0);
    float4 thisVert = (vertIn.vertexId % 2 == 0) ? lineStart : lineEnd;

    float3 invDirectionToCam = thisVert.xyz - camWorldPosition;

    float4 diff = lineEnd - lineStart;
    float4 norm = normalize(float4(cross(diff, float4(-invDirectionToCam, 1.0)), 0.0));

    // Can we do this with UVs instead?
    if (vertIn.vertexId == 0)
        thisVert += norm * thickness;
    else if (vertIn.vertexId == 1)
        thisVert += norm * thickness;
    else if (vertIn.vertexId == 2)
        thisVert -= norm * thickness;
    else if (vertIn.vertexId == 3)
        thisVert -= norm * thickness;

    output.pos = mul(thisVert, worldToClipTransform);
    output.col = vertIn.col;
    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    return pixelIn.col;
}