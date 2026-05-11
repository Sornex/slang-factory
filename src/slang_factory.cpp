#include "slang_factory/slang_factory.h"

#include <sstream>

static void append_line(std::string& out, const std::string& line = "")
{
    out += line;
    out += "\n";
}

std::string SlangFactory::generate_pipeline_module(const ShaderRequest& req) const
{
    std::string out;

    emit_common_types(req, out);

    if (req.mode == ShaderPipelineMode::VertexOnly ||
        req.mode == ShaderPipelineMode::VertexFragment)
    {
        emit_vertex_entry(req, out);
    }

    if (req.mode == ShaderPipelineMode::FragmentOnly ||
        req.mode == ShaderPipelineMode::VertexFragment)
    {
        emit_fragment_entry(req, out);
    }

    return out;
}

void SlangFactory::emit_common_types(const ShaderRequest& req, std::string& out) const
{
    // Vertex input structure
    append_line(out, "struct VSInput");
    append_line(out, "{");

    for (const auto& a : req.vertex_inputs)
    {
        append_line(out, "    " + a.type + " " + a.name + " : " + a.semantic + ";");
    }

    append_line(out, "};");
    append_line(out);

    // Vertex output / fragment input structure
    append_line(out, "struct VSOutput");
    append_line(out, "{");
    append_line(out, "    float4 position : SV_Position;");

    if (req.use_uv)
        append_line(out, "    float2 uv : TEXCOORD0;");

    if (req.use_vertex_color)
        append_line(out, "    float4 color : COLOR0;");

    append_line(out, "};");
    append_line(out);

    // For now FSInput is the same as VSOutput.
    // This makes VS -> FS varyings consistent.
    append_line(out, "typedef VSOutput FSInput;");
    append_line(out);

    // Vertex shader uniform data
    append_line(out, "cbuffer CameraParams");
    append_line(out, "{");
    append_line(out, "    float4x4 mvp;");
    append_line(out, "};");
    append_line(out);

    // Fragment shader uniform data
    if (req.use_constant_color)
    {
        append_line(out, "cbuffer MaterialParams");
        append_line(out, "{");
        append_line(out, "    float4 base_color;");
        append_line(out, "};");
        append_line(out);
    }

    // Fragment shader resources
    if (req.use_texture)
    {
        append_line(out, "Texture2D<float4> albedo_tex;");
        append_line(out, "SamplerState samp;");
        append_line(out);
    }
}

void SlangFactory::emit_vertex_entry(const ShaderRequest& req, std::string& out) const
{
    append_line(out, "[shader(\"vertex\")]");
    append_line(out, "VSOutput " + req.vs_entry + "(VSInput input)");
    append_line(out, "{");
    append_line(out, "    VSOutput o;");
    append_line(out, "    o.position = mul(mvp, float4(input.position, 1.0));");

    if (req.use_uv)
        append_line(out, "    o.uv = input.uv;");

    if (req.use_vertex_color)
        append_line(out, "    o.color = input.color;");

    append_line(out, "    return o;");
    append_line(out, "}");
    append_line(out);
}

void SlangFactory::emit_fragment_entry(const ShaderRequest& req, std::string& out) const
{
    append_line(out, "[shader(\"fragment\")]");
    append_line(out, "float4 " + req.fs_entry + "(FSInput input) : SV_Target0");
    append_line(out, "{");
    append_line(out, "    float4 c = float4(0, 0, 0, 1);");

    if (req.use_texture)
    {
        append_line(out, "    c = albedo_tex.Sample(samp, input.uv);");
    }

    if (req.use_vertex_color)
    {
        if (req.use_texture)
            append_line(out, "    c *= input.color;");
        else
            append_line(out, "    c = input.color;");
    }

    if (req.use_constant_color)
    {
        if (req.use_texture || req.use_vertex_color)
            append_line(out, "    c *= base_color;");
        else
            append_line(out, "    c = base_color;");
    }

    append_line(out, "    return c;");
    append_line(out, "}");
    append_line(out);
}