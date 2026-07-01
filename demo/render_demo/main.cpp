#include <iostream>

#include "helpers/shader_request.h"
#include "slang_factory/slang_shader_builder.h"
#include "vulkan_renderer.h"

namespace
{
    constexpr int k_window_width = 1280;
    constexpr int k_window_height = 720;
    constexpr const char* k_window_title = "Slang Factory Render Demo";

    ShaderRequest make_render_demo_request()
    {
        ShaderRequest req;
        req.mode = ShaderPipelineMode::VertexFragment;
        req.vs_entry = "vertexMain";
        req.fs_entry = "fragmentMain";

        req.vertex_inputs.push_back({ "position", "float3", "POSITION" });
        req.vertex_inputs.push_back({ "color", "float4", "COLOR0" });

        req.use_uv = false;
        req.use_vertex_color = true;
        req.use_texture = false;
        req.use_constant_color = true;
        req.use_mvp_transform = true;

        return req;
    }

    void print_stage_diagnostics(const char* label, const SlangShaderBuilder::StageResult& stage)
    {
        if (stage.diagnostics_text.empty())
            return;

        std::cerr << "\n=== " << label << " Diagnostics ===\n";
        std::cerr << stage.diagnostics_text << "\n";
    }
    
    static DemoScene read_demo_scene()
    {
        std::cout << "Choose render demo scene:\n";
        std::cout << "1) Triangle\n";
        std::cout << "2) Quad\n";
        std::cout << "3) Animated Triangle\n";
        std::cout << "Choice: ";

        int choice = 1;
        std::cin >> choice;

        switch (choice)
        {
        case 2: return DemoScene::Quad;
        case 3: return DemoScene::AnimatedTriangle;
        default: return DemoScene::Triangle;
        }
    }
}


int main()
{
    const DemoScene scene = read_demo_scene();
    const ShaderRequest request = make_render_demo_request();
    
    SlangShaderBuilder builder;
    if (!builder.init_spirv_1_5())
    {
        std::cerr << "Failed to initialize SlangShaderBuilder.\n";
        return -1;
    }

    const auto build_result = builder.build_pipeline(request);
    if (!build_result.has_valid_pipeline())
    {
        std::cerr << "Failed to build shader pipeline.\n";
        print_stage_diagnostics("Vertex", build_result.vertex);
        print_stage_diagnostics("Fragment", build_result.fragment);
        return -1;
    }

    std::cout << "Shader pipeline built successfully.\n";
    std::cout << "Vertex SPIR-V bytes: " << build_result.vertex.target_code->getBufferSize() << "\n";
    std::cout << "Fragment SPIR-V bytes: " << build_result.fragment.target_code->getBufferSize() << "\n";

    RendererShaderBinaries shaders;
    shaders.vertex_shader = build_result.vertex.target_code.get();
    shaders.fragment_shader = build_result.fragment.target_code.get();

    VulkanRenderer renderer;
    if (!renderer.init(k_window_title, k_window_width, k_window_height, shaders, scene))
    {
        std::cerr << "Failed to initialize VulkanRenderer.\n";
        return -1;
    }

    renderer.run();
    return 0;
}