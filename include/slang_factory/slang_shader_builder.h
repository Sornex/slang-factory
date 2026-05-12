#pragma once

#include <string>

#include <slang.h>
#include <slang-com-ptr.h>

#include "helpers/shader_request.h"
#include "helpers/slang_compiler.h"
#include "slang_factory/slang_factory.h"

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
    };

    struct BuildResult
    {
        bool success = false;

        std::string generated_source;

        StageResult vertex;
        StageResult fragment;
    };

public:
    SlangShaderBuilder() = default;

    bool init_spirv_1_5();

    BuildResult build_pipeline(const ShaderRequest& request);

private:
    SlangCompiler compiler;
    SlangFactory factory;
};