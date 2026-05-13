#include "slang_factory/slang_shader_builder.h"
#include <fstream>

bool SlangShaderBuilder::init_spirv_1_5()
{
    return compiler.init_spirv_1_5();
}

std::string SlangShaderBuilder::blob_to_string(slang::IBlob* blob)
{
    if (!blob) return "";

    const char* text = static_cast<const char*>(blob->getBufferPointer());
    if (!text) return "";

    return std::string(text, blob->getBufferSize());
}

bool SlangShaderBuilder::write_text_file(const char* path, const std::string& text)
{
    if (!path) return false;

    std::ofstream file(path);
    if (!file) return false;

    file << text;
    return static_cast<bool>(file);
}

bool SlangShaderBuilder::write_blob_to_file(const char* path, slang::IBlob* blob)
{
    if (!path || !blob) return false;

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    file.write(
        static_cast<const char*>(blob->getBufferPointer()),
        static_cast<std::streamsize>(blob->getBufferSize()));

    return static_cast<bool>(file);
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
    result.vertex.diagnostics_text = blob_to_string(vs_result.diagnostics.get());
    result.vertex.reflection_data = extractor.extract(vs_result.layout);
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
    result.fragment.diagnostics_text = blob_to_string(fs_result.diagnostics.get());
    result.fragment.reflection_data = extractor.extract(fs_result.layout);
    result.fragment.success = (fs_result.layout != nullptr && fs_result.target_code != nullptr);

    result.success = result.has_valid_pipeline();
    return result;
}