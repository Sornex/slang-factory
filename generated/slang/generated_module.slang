struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

typedef VSOutput FSInput;

cbuffer CameraParams
{
    float4x4 mvp;
};

cbuffer MaterialParams
{
    float4 base_color;
};

Texture2D<float4> albedo_tex;
SamplerState samp;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput o;
    o.position = mul(mvp, float4(input.position, 1.0));
    o.uv = input.uv;
    o.color = input.color;
    return o;
}

[shader("fragment")]
float4 fragmentMain(FSInput input) : SV_Target0
{
    float4 c = float4(0, 0, 0, 1);
    c = albedo_tex.Sample(samp, input.uv);
    c *= input.color;
    c *= base_color;
    return c;
}

