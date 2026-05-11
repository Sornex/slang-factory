#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "helpers/slang_compiler.h"
#include "helpers/slang_reflection_dumper.h"
#include "slang_factory/slang_factory.h"
#include "helpers/shader_request.h"

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

static void compile_and_dump(
    SlangCompiler& compiler,
    const std::string& source,
    const char* entry_point,
    const char* output_spv_path)
{
    auto result = compiler.compile_from_source_string(
        "generated_module",
        "generated_module.slang",
        source.c_str(),
        entry_point);

    if (result.diagnostics)
    {
        print_diagnostics(result.diagnostics.get());
    }

    if (result.target_code)
    {
        std::cout << "[" << entry_point << "] SPIR-V bytes: "
                  << result.target_code->getBufferSize() << "\n";

        if (compiler.write_target_code_to_file(output_spv_path, result.target_code.get()))
        {
            std::cout << "[" << entry_point << "] wrote: " << output_spv_path << "\n";
        }
        else
        {
            std::cerr << "[" << entry_point << "] failed to write: " << output_spv_path << "\n";
        }
    }
    else
    {
        std::cerr << "[" << entry_point << "] no target code produced.\n";
    }

    if (!result.layout)
    {
        std::cerr << "[" << entry_point << "] no ProgramLayout returned.\n";
        return;
    }

    std::cout << "\n=== Reflection Dump (" << entry_point << ") ===\n";
    SlangReflectionDumper dumper(std::cout);
    dumper.dump(result.layout);
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

    SlangFactory factory;
    std::string source = factory.generate_pipeline_module(req);

    std::filesystem::create_directories("generated/slang");
    std::filesystem::create_directories("generated/shaders");

    const char* slang_path = "generated/slang/generated_module.slang";
    if (write_text_file(slang_path, source))
    {
        std::cout << "Wrote Slang source to: " << slang_path << "\n";
    }
    else
    {
        std::cerr << "Failed to write Slang source to: " << slang_path << "\n";
    }

    std::cout << "\n=== Generated Slang Module ===\n";
    std::cout << source << "\n";

    SlangCompiler compiler;
    if (!compiler.init_spirv_1_5())
    {
        std::cerr << "Failed to initialize SlangCompiler.\n";
        return -1;
    }

    compile_and_dump(
        compiler,
        source,
        req.vs_entry.c_str(),
        "generated/shaders/vertex.spv");

    compile_and_dump(
        compiler,
        source,
        req.fs_entry.c_str(),
        "generated/shaders/fragment.spv");

    return 0;
}