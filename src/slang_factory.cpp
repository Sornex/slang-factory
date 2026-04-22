#include "slang_factory/slang_factory.h"
#include <sstream>

static void line(std::ostringstream& ss, const std::string& s = "")
{
    ss << s << "\n";
}

std::string SlangFactory::generate(const ShaderRequest& req) const
{
    switch (req.stage)
    {
    case ShaderStage::Vertex: return generate_vertex_shader(req);
    case ShaderStage::Fragment: return generate_fragment_shader(req);
    default: return "";
    }
}

std::string SlangFactory::generate_vertex_shader(const ShaderRequest& req) const
{
    std::ostringstream ss;

    // Types
    line(ss, "struct VSInput");
    line(ss, "{");
    for (const auto& a : req.vertex_inputs)
    {
        line(ss, "    " + a.type + " " + a.name + " : " + a.semantic + ";");
    }
    line(ss, "};");
    line(ss);

    line(ss, "struct VSOutput");
    line(ss, "{");
    line(ss, "    float4 position : SV_Position;");
    if (req.pass_uv_to_fragment)
        line(ss, "    float2 uv : TEXCOORD0;");
    if (req.pass_color_to_fragment)
        line(ss, "    float4 color : COLOR0;");
    line(ss, "};");
    line(ss);

    // Uniforms
    line(ss, "cbuffer CameraParams");
    line(ss, "{");
    line(ss, "    float4x4 mvp;");
    line(ss, "};");
    line(ss);

    // Entry point
    line(ss, "[shader(\"vertex\")]");
    line(ss, "VSOutput " + req.entry_point + "(VSInput input)");
    line(ss, "{");
    line(ss, "    VSOutput o;");

    line(ss, "    o.position = mul(mvp, float4(input.position, 1.0));");

    if (req.pass_uv_to_fragment)
        line(ss, "    o.uv = input.uv;");

    if (req.pass_color_to_fragment)
        line(ss, "    o.color = input.color;");

    line(ss, "    return o;");
    line(ss, "}");

    return ss.str();
}

std::string SlangFactory::generate_fragment_shader(const ShaderRequest& req) const
{
    std::ostringstream ss;

    line(ss, "struct FSInput");
    line(ss, "{");
    line(ss, "    float4 position : SV_Position;");
    if (req.use_texture)
        line(ss, "    float2 uv : TEXCOORD0;");
    if (req.use_vertex_color)
        line(ss, "    float4 color : COLOR0;");
    line(ss, "};");
    line(ss);

    if (req.use_constant_color)
    {
        line(ss, "cbuffer MaterialParams");
        line(ss, "{");
        line(ss, "    float4 base_color;");
        line(ss, "};");
        line(ss);
    }

    if (req.use_texture)
    {
        line(ss, "Texture2D<float4> albedo_tex;");
        line(ss, "SamplerState samp;");
        line(ss);
    }

    line(ss, "[shader(\"fragment\")]");
    line(ss, "float4 " + req.entry_point + "(FSInput input) : SV_Target0");
    line(ss, "{");

    line(ss, "    float4 c = float4(0,0,0,1);");

    if (req.use_texture)
    {
        line(ss, "    c = albedo_tex.Sample(samp, input.uv);");
    }

    if (req.use_vertex_color)
    {
        if (req.use_texture)
            line(ss, "    c *= input.color;");
        else
            line(ss, "    c = input.color;");
    }

    if (req.use_constant_color)
    {
        if (req.use_texture || req.use_vertex_color)
            line(ss, "    c *= base_color;");
        else
            line(ss, "    c = base_color;");
    }

    line(ss, "    return c;");
    line(ss, "}");

    return ss.str();
}