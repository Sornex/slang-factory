#pragma once
#include <string>
#include <vector>

// Defines which shader stages should be generated.
enum class ShaderPipelineMode
{
    VertexOnly,
    FragmentOnly,
    VertexFragment
};

struct VertexAttribute
{
    std::string name;      // position, uv, color
    std::string type;      // float3, float2, float4
    std::string semantic;  // POSITION, TEXCOORD0, COLOR0
};

// Describes what kind of shader pipeline the factory should generate.
struct ShaderRequest
{
    ShaderPipelineMode mode = ShaderPipelineMode::VertexFragment;

    // Entry point names
    std::string vs_entry = "vertexMain";
    std::string fs_entry = "fragmentMain";

    // Vertex inputs
    std::vector<VertexAttribute> vertex_inputs;

    // Varyings to pass VS -> FS
    bool use_uv = false;
    bool use_vertex_color = false;

    // Fragment shading options
    bool use_constant_color = true;
    bool use_texture = false;

    bool use_mvp_transform = true;
};