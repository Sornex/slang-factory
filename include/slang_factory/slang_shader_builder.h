#pragma once

#include <string>

#include <slang.h>
#include <slang-com-ptr.h>

#include "helpers/shader_request.h"
#include "helpers/slang_compiler.h"
#include "slang_factory/slang_factory.h"
#include "helpers/slang_reflection_extractor.h"

class SlangShaderBuilder
{
public:
    struct StageResult
    {
        bool success = false;

        Slang::ComPtr<slang::IComponentType> linked_program;
        slang::ProgramLayout* layout = nullptr;

        Slang::ComPtr<slang::IBlob> target_code;
        Slang::ComPtr<slang::IBlob> diagnostics;
        std::string diagnostics_text;
        
        ReflectedProgram reflection_data;

        bool has_code() const { return target_code != nullptr; }
        bool has_layout() const { return layout != nullptr; }
    };

    struct BuildResult
    {
        bool success = false;

        std::string generated_source;

        StageResult vertex;
        StageResult fragment;

        bool has_valid_pipeline() const
        {
            return vertex.success && fragment.success &&
                   vertex.has_code() && fragment.has_code() &&
                   vertex.has_layout() && fragment.has_layout();
        }
    };

    SlangShaderBuilder() = default;

    bool init_spirv_1_5();

    BuildResult build_pipeline(const ShaderRequest& request);

    static bool write_text_file(const char* path, const std::string& text);
    static bool write_blob_to_file(const char* path, slang::IBlob* blob);
private:
    SlangCompiler compiler;
    SlangFactory factory;
    SlangReflectionExtractor extractor;

    static std::string blob_to_string(slang::IBlob* blob);
};