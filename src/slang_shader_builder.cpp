#include "slang_factory/slang_shader_builder.h"

bool SlangShaderBuilder::init_spirv_1_5()
{
    return compiler.init_spirv_1_5();
}

SlangShaderBuilder::BuildResult SlangShaderBuilder::build_pipeline(const ShaderRequest& request)
{
    BuildResult result;

    result.generated_source = factory.generate_pipeline_module(request);

    // Compile vertex shader
    auto vs_result = compiler.compile_from_source_string(
        "generated_module",
        "generated_module.slang",
        result.generated_source.c_str(),
        request.vs_entry.c_str());

    result.vertex.linked_program = vs_result.linked_program;
    result.vertex.layout = vs_result.layout;
    result.vertex.target_code = vs_result.target_code;
    result.vertex.diagnostics = vs_result.diagnostics;
    result.vertex.success = (vs_result.layout != nullptr && vs_result.target_code != nullptr);

    // Compile fragment shader
    auto fs_result = compiler.compile_from_source_string(
        "generated_module",
        "generated_module.slang",
        result.generated_source.c_str(),
        request.fs_entry.c_str());

    result.fragment.linked_program = fs_result.linked_program;
    result.fragment.layout = fs_result.layout;
    result.fragment.target_code = fs_result.target_code;
    result.fragment.diagnostics = fs_result.diagnostics;
    result.fragment.success = (fs_result.layout != nullptr && fs_result.target_code != nullptr);

    result.success = result.vertex.success && result.fragment.success;
    return result;
}