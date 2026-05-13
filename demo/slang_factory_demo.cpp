#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "helpers/slang_reflection_dumper.h"
#include "helpers/shader_request.h"
#include "slang_factory/slang_shader_builder.h"

static int read_int_choice()
{
    int v = 0;
    std::cin >> v;
    return v;
}

static void print_extracted_field(const ReflectedField& field, int depth = 0)
{
    for (int i = 0; i < depth; ++i) std::cout << "  ";

    std::cout << "- name: " << field.name
              << ", type: " << field.type_name
              << ", kind: " << field.kind << "\n";

    if (!field.offsets.empty())
    {
        for (int i = 0; i < depth + 1; ++i) std::cout << "  ";
        std::cout << "offsets:\n";

        for (const auto& off : field.offsets)
        {
            for (int i = 0; i < depth + 2; ++i) std::cout << "  ";
            std::cout << off.category
                      << " offset=" << off.offset
                      << " space=" << off.space << "\n";
        }
    }

    if (!field.children.empty())
    {
        for (int i = 0; i < depth + 1; ++i) std::cout << "  ";
        std::cout << "children:\n";

        for (const auto& child : field.children)
        {
            print_extracted_field(child, depth + 2);
        }
    }
}

static void print_extracted_program(const ReflectedProgram& program, const char* label)
{
    std::cout << "\n=== Extracted Reflection (" << label << ") ===\n";

    std::cout << "Global fields:\n";
    for (const auto& field : program.global_fields)
    {
        print_extracted_field(field, 1);
    }

    std::cout << "Entry points:\n";
    for (const auto& ep : program.entry_points)
    {
        std::cout << "  entry: " << ep.entry_name
                  << ", stage: " << ep.stage << "\n";

        std::cout << "    parameters:\n";
        for (const auto& p : ep.parameters)
        {
            print_extracted_field(p, 3);
        }

        std::cout << "    result:\n";
        for (const auto& r : ep.result_fields)
        {
            print_extracted_field(r, 3);
        }
    }
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
    if (SlangShaderBuilder::write_text_file(slang_path, build_result.generated_source))
    {
        std::cout << "Wrote Slang source to: " << slang_path << "\n";
    }
    else
    {
        std::cerr << "Failed to write Slang source to: " << slang_path << "\n";
    }

    std::cout << "\n=== Generated Slang Module ===\n";
    std::cout << build_result.generated_source << "\n";

    if (!build_result.vertex.diagnostics_text.empty())
    {
        std::cout << "\n=== Vertex Diagnostics ===\n";
        std::cout << build_result.vertex.diagnostics_text << "\n";
    }

    if (!build_result.fragment.diagnostics_text.empty())
    {
        std::cout << "\n=== Fragment Diagnostics ===\n";
        std::cout << build_result.fragment.diagnostics_text << "\n";
    }

    if (build_result.vertex.target_code)
    {
        const char* vertex_path = "generated/shaders/vertex.spv";
        if (SlangShaderBuilder::write_blob_to_file(vertex_path, build_result.vertex.target_code.get()))
        {
            std::cout << "Wrote vertex SPIR-V to: " << vertex_path << "\n";
            std::cout << "Vertex SPIR-V bytes: "
                      << build_result.vertex.target_code->getBufferSize() << "\n";
        }
    }

    if (build_result.fragment.target_code)
    {
        const char* fragment_path = "generated/shaders/fragment.spv";
        if (SlangShaderBuilder::write_blob_to_file(fragment_path, build_result.fragment.target_code.get()))
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

    print_extracted_program(build_result.vertex.reflection_data, "Vertex");
    print_extracted_program(build_result.fragment.reflection_data, "Fragment");

    const auto* camera = SlangReflectionExtractor::find_field_by_name(
    build_result.vertex.reflection_data.global_fields,
    "CameraParams");

    if (camera)
    {
        std::cout << "\n[TEST] Found CameraParams\n";
    }

    const auto* mvp = SlangReflectionExtractor::find_field_by_path(
        build_result.vertex.reflection_data.global_fields,
        {"CameraParams", "mvp"});

    if (mvp)
    {
        std::cout << "[TEST] Found CameraParams.mvp, type=" << mvp->type_name
                << ", kind=" << mvp->kind << "\n";
    }

    std::vector<const ReflectedField*> uniform_fields;
    SlangReflectionExtractor::flatten_uniform_fields(
        build_result.vertex.reflection_data.global_fields,
        uniform_fields);

    std::cout << "[TEST] Flattened uniform fields count: " << uniform_fields.size() << "\n";
    for (const auto* f : uniform_fields)
    {
        std::cout << "  - " << f->name << " (" << f->kind << ")\n";
    }

    auto vertex_uniforms = SlangReflectionExtractor::get_uniform_leaf_fields(build_result.vertex.reflection_data);

    std::cout << "\n[TEST] Vertex uniform leaf fields:\n";
    for (const auto* f : vertex_uniforms)
    {
        std::cout << "  - " << f->name << " (" << f->kind << ")\n";
    }

    auto fragment_resources = SlangReflectionExtractor::get_resource_fields(build_result.fragment.reflection_data);

    std::cout << "[TEST] Fragment resources:\n";
    for (const auto* f : fragment_resources)
    {
        std::cout << "  - " << f->name << " (" << f->kind << ")\n";
    }

    if (!build_result.vertex.reflection_data.entry_points.empty())
    {
        const auto& vertex_stage = build_result.vertex.reflection_data.entry_points[0];
        auto stage_inputs = SlangReflectionExtractor::get_stage_parameter_fields(vertex_stage);
        auto stage_outputs = SlangReflectionExtractor::get_stage_result_fields(vertex_stage);

        std::cout << "[TEST] Vertex stage inputs:\n";
        for (const auto* f : stage_inputs)
        {
            std::cout << "  - " << f->name << " (" << f->kind << ")\n";
        }

        std::cout << "[TEST] Vertex stage outputs:\n";
        for (const auto* f : stage_outputs)
        {
            std::cout << "  - " << f->name << " (" << f->kind << ")\n";
        }
    }

    if (!build_result.fragment.reflection_data.entry_points.empty())
    {
        const auto& fragment_stage = build_result.fragment.reflection_data.entry_points[0];
        auto stage_inputs = SlangReflectionExtractor::get_stage_parameter_fields(fragment_stage);
        auto stage_outputs = SlangReflectionExtractor::get_stage_result_fields(fragment_stage);

        std::cout << "[TEST] Fragment stage inputs:\n";
        for (const auto* f : stage_inputs)
        {
            std::cout << "  - " << f->name << " (" << f->kind << ")\n";
        }

        std::cout << "[TEST] Fragment stage outputs:\n";
        for (const auto* f : stage_outputs)
        {
            std::cout << "  - " << f->name << " (" << f->kind << ")\n";
        }
    }

    if (!build_result.has_valid_pipeline())
    {
        std::cerr << "\nBuild pipeline failed.\n";
        return -1;
    }

    std::cout << "\nBuild pipeline succeeded.\n";
    return 0;
}