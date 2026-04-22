#pragma once
#include <string>
#include <vector>

enum class ShaderStage
{
    Vertex,
    Fragment
};

struct VertexAttribute
{
    std::string name;      // position, uv, color
    std::string type;      // float3, float2, float4
    std::string semantic;  // POSITION, TEXCOORD0, COLOR0
};

struct ShaderRequest
{
    ShaderStage stage = ShaderStage::Vertex;

    // Common
    std::string entry_point = "main";

    // Vertex shader settings
    std::vector<VertexAttribute> vertex_inputs;
    bool pass_uv_to_fragment = false;
    bool pass_color_to_fragment = false;

    // Fragment shader settings
    bool use_constant_color = true;
    bool use_vertex_color = false;
    bool use_texture = false;
    
};