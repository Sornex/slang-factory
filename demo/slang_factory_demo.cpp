#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "helpers/slang_reflection_dumper.h"
#include "helpers/shader_request.h"
#include "slang_factory/slang_shader_builder.h"

static void print_diagnostics(slang::IBlob* diagnostics)
{
    if (!diagnostics) return;
    const char* text = (const char*)diagnostics->getBufferPointer();
    if (text && text[0]) std::cerr << text << "\n";
}

static int read_int_choice()
{
    int v = 0;
    std::cin >> v;
    return v;
}

static bool write_text_file(const char* path, const std::string& text)
{
    if (!path) return false;

    std::ofstream file(path);
    if (!file) return false;

    file << text;
    return (bool)file;
}

static bool write_blob_to_file(const char* path, slang::IBlob* blob)
{
    if (!path || !blob) return false;

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    file.write(
        static_cast<const char*>(blob->getBufferPointer()),
        static_cast<std::streamsize>(blob->getBufferSize()));

    return static_cast<bool>(file);
}

int main()
{
    std::cout << "=== Slang VS + FS Wizard ===\n";

    ShaderRequest req;
    req.mode = ShaderPipelineMode::VertexFragment;
    req.vs_entry = "vertexMain";
    req.fs_entry = "fragmentMain";

    // Vertex position is required for the vertex shader.
    req.vertex_inputs.push_back({ "position", "float3", "POSITION" });

    std::cout << "Use UV (TEXCOORD0)? 1=yes 0=no: ";
    req.use_uv = (read_int_choice() == 1);
    if (req.use_uv)
    {
        req.vertex_inputs.push_back({ "uv", "float2", "TEXCOORD0" });
    }

    std::cout << "Use vertex color (COLOR0)? 1=yes 0=no: ";
    req.use_vertex_color = (read_int_choice() == 1);
    if (req.use_vertex_color)
    {
        req.vertex_inputs.push_back({ "color", "float4", "COLOR0" });
    }

    std::cout << "Fragment shader: use texture? 1=yes 0=no: ";
    req.use_texture = (read_int_choice() == 1);

    std::cout << "Fragment shader: use constant color? 1=yes 0=no: ";
    req.use_constant_color = (read_int_choice() == 1);

    // Texture sampling requires UVs. If user forgot UV, enable it automatically.
    if (req.use_texture && !req.use_uv)
    {
        std::cout << "Texture requires UV, enabling UV automatically.\n";

        req.use_uv = true;

        bool has_uv = false;
        for (const auto& input : req.vertex_inputs)
        {
            if (input.name == "uv")
            {
                has_uv = true;
                break;
            }
        }

        if (!has_uv)
        {
            req.vertex_inputs.push_back({ "uv", "float2", "TEXCOORD0" });
        }
    }

    std::filesystem::create_directories("generated/slang");
    std::filesystem::create_directories("generated/shaders");

    SlangShaderBuilder builder;

    if (!builder.init_spirv_1_5())
    {
        std::cerr << "Failed to initialize SlangShaderBuilder.\n";
        return -1;
    }

    auto build_result = builder.build_pipeline(req);

    const char* slang_path = "generated/slang/generated_module.slang";
    if (write_text_file(slang_path, build_result.generated_source))
    {
        std::cout << "Wrote Slang source to: " << slang_path << "\n";
    }
    else
    {
        std::cerr << "Failed to write Slang source to: " << slang_path << "\n";
    }

    std::cout << "\n=== Generated Slang Module ===\n";
    std::cout << build_result.generated_source << "\n";

    if (build_result.vertex.diagnostics)
    {
        std::cout << "\n=== Vertex Diagnostics ===\n";
        print_diagnostics(build_result.vertex.diagnostics.get());
    }

    if (build_result.fragment.diagnostics)
    {
        std::cout << "\n=== Fragment Diagnostics ===\n";
        print_diagnostics(build_result.fragment.diagnostics.get());
    }

    if (build_result.vertex.target_code)
    {
        const char* vertex_path = "generated/shaders/vertex.spv";
        if (write_blob_to_file(vertex_path, build_result.vertex.target_code.get()))
        {
            std::cout << "Wrote vertex SPIR-V to: " << vertex_path << "\n";
            std::cout << "Vertex SPIR-V bytes: "
                      << build_result.vertex.target_code->getBufferSize() << "\n";
        }
    }

    if (build_result.fragment.target_code)
    {
        const char* fragment_path = "generated/shaders/fragment.spv";
        if (write_blob_to_file(fragment_path, build_result.fragment.target_code.get()))
        {
            std::cout << "Wrote fragment SPIR-V to: " << fragment_path << "\n";
            std::cout << "Fragment SPIR-V bytes: "
                      << build_result.fragment.target_code->getBufferSize() << "\n";
        }
    }

    if (build_result.vertex.layout)
    {
        std::cout << "\n=== Reflection Dump (Vertex) ===\n";
        SlangReflectionDumper dumper(std::cout);
        dumper.dump(build_result.vertex.layout);
    }
    else
    {
        std::cerr << "No vertex ProgramLayout returned.\n";
    }

    if (build_result.fragment.layout)
    {
        std::cout << "\n=== Reflection Dump (Fragment) ===\n";
        SlangReflectionDumper dumper(std::cout);
        dumper.dump(build_result.fragment.layout);
    }
    else
    {
        std::cerr << "No fragment ProgramLayout returned.\n";
    }

    if (!build_result.success)
    {
        std::cerr << "\nBuild pipeline failed.\n";
        return -1;
    }

    std::cout << "\nBuild pipeline succeeded.\n";
    return 0;
}