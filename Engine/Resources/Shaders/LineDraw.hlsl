
cbuffer PerObjectData
{
    // TODO: need to pass in view/projection matrix, effectively world to clip transform
    float4x4 WVP;
};

struct VertInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
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
    output.pos = mul(vertIn.pos, WVP);
    output.col = vertIn.col;
    return output;
}

float4 PSMain(VertOutput pixelIn) : SV_TARGET
{
    return pixelIn.col;
}