#include <iostream>

#include "helpers/shader_request.h"
#include "slang_factory/slang_shader_builder.h"
#include "vulkan_renderer.h"

int main()
{
    // Build a very simple pipeline for the renderer:
    // - vertex input: position + color
    // - fragment: vertex color only
    ShaderRequest req;
    req.mode = ShaderPipelineMode::VertexFragment;
    req.vs_entry = "vertexMain";
    req.fs_entry = "fragmentMain";

    req.vertex_inputs.push_back({ "position", "float3", "POSITION" });
    req.vertex_inputs.push_back({ "color", "float4", "COLOR0" });

    req.use_uv = false;
    req.use_vertex_color = true;
    req.use_texture = false;
    req.use_constant_color = false;
    req.use_mvp_transform = false;

    SlangShaderBuilder builder;
    if (!builder.init_spirv_1_5())
    {
        std::cerr << "Failed to initialize SlangShaderBuilder.\n";
        return -1;
    }

    auto build_result = builder.build_pipeline(req);
    if (!build_result.has_valid_pipeline())
    {
        std::cerr << "Failed to build shader pipeline.\n";

        if (!build_result.vertex.diagnostics_text.empty())
        {
            std::cerr << "\n=== Vertex Diagnostics ===\n";
            std::cerr << build_result.vertex.diagnostics_text << "\n";
        }

        if (!build_result.fragment.diagnostics_text.empty())
        {
            std::cerr << "\n=== Fragment Diagnostics ===\n";
            std::cerr << build_result.fragment.diagnostics_text << "\n";
        }

        return -1;
    }

    std::cout << "Shader pipeline built successfully.\n";
    std::cout << "Vertex SPIR-V bytes: " << build_result.vertex.target_code->getBufferSize() << "\n";
    std::cout << "Fragment SPIR-V bytes: " << build_result.fragment.target_code->getBufferSize() << "\n";

    RendererShaderBinaries shaders;
    shaders.vertex_shader = build_result.vertex.target_code.get();
    shaders.fragment_shader = build_result.fragment.target_code.get();

    VulkanRenderer renderer;
    if (!renderer.init("Slang Factory Render Demo", 1280, 720, shaders))
    {
        std::cerr << "Failed to initialize VulkanRenderer.\n";
        return -1;
    }

    renderer.run();
    return 0;
}