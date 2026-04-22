#include <iostream>
#include <string>

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

int main()
{
    std::cout << "=== Slang Shader Wizard ===\n";
    std::cout << "1) Vertex shader\n";
    std::cout << "2) Fragment shader\n";
    std::cout << "Choose: ";

    int choice = read_int_choice();

    ShaderRequest req;
    req.entry_point = "main";

    if (choice == 1)
    {
        req.stage = ShaderStage::Vertex;

        req.vertex_inputs.push_back({ "position", "float3", "POSITION" });

        std::cout << "Include UV (TEXCOORD0)? 1=yes 0=no: ";
        int has_uv = read_int_choice();
        if (has_uv)
        {
            req.vertex_inputs.push_back({ "uv", "float2", "TEXCOORD0" });
            req.pass_uv_to_fragment = true;
        }

        std::cout << "Include vertex color (COLOR0)? 1=yes 0=no: ";
        int has_col = read_int_choice();
        if (has_col)
        {
            req.vertex_inputs.push_back({ "color", "float4", "COLOR0" });
            req.pass_color_to_fragment = true;
        }
    }
    else if (choice == 2)
    {
        req.stage = ShaderStage::Fragment;

        std::cout << "Color source:\n";
        std::cout << "1) Constant color (cbuffer)\n";
        std::cout << "2) Vertex color\n";
        std::cout << "3) Texture\n";
        std::cout << "4) Texture * Vertex color\n";
        std::cout << "Choose: ";

        int mode = read_int_choice();

        req.use_constant_color = (mode == 1);
        req.use_vertex_color   = (mode == 2 || mode == 4);
        req.use_texture        = (mode == 3 || mode == 4);
    }
    else
    {
        std::cerr << "Unknown choice.\n";
        return -1;
    }

    // Generate Slang code
    SlangFactory factory;
    std::string source = factory.generate(req);

    std::cout << "\n=== Generated Slang ===\n";
    std::cout << source << "\n";

    // Compile + reflection + SPIR-V
    SlangCompiler compiler;
    if (!compiler.init_spirv_1_5())
    {
        std::cerr << "Failed to initialize SlangCompiler.\n";
        return -1;
    }

    auto result = compiler.compile_from_source_string(
        "generated_module",
        "generated_module.slang",
        source.c_str(),
        req.entry_point.c_str());

    if (result.diagnostics)
        print_diagnostics(result.diagnostics.get());

    if (result.target_code)
    {
        std::cout << "Compiled SPIR-V bytes: " << result.target_code->getBufferSize() << "\n";
        const char* out_path = "generated/generated_module.spv";
        if (compiler.write_target_code_to_file(out_path, result.target_code.get()))
            std::cout << "Wrote SPIR-V to: " << out_path << "\n";
        else
            std::cout << "Failed to write SPIR-V to: " << out_path << "\n";
    }

    if (!result.layout)
    {
        std::cerr << "No ProgramLayout returned.\n";
        return -1;
    }

    std::cout << "\n=== Reflection Dump ===\n";
    SlangReflectionDumper dumper(std::cout);
    dumper.dump(result.layout);

    return 0;
}