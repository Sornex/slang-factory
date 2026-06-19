#pragma once

#include <string>

#include <slang.h>
#include <slang-com-ptr.h>

// I was following a Using the Compilation API tutorial here https://docs.shader-slang.org/en/latest/compilation-api.html

class SlangCompiler
{
public:
    // Stores the main outputs produced by one Slang compilation request.
    struct Result
    {
        Slang::ComPtr<slang::IComponentType> linked_program;
        slang::ProgramLayout* layout = nullptr;
        Slang::ComPtr<slang::IBlob> target_code;

        Slang::ComPtr<slang::IBlob> diagnostics;
    };

    // Creates global session and session configured for one target (SPIR-V for now as I will need it for Vulkan).
    // Returns false if initialization fails.
    bool init_spirv_1_5();

    // Compiles a module from a source string, links, and returns ProgramLayout.
    Result compile_from_source_string(
        const char* module_name,
        const char* virtual_path,
        const char* source,
        const char* entry_point_name);

    // Writes compiled target code (for example SPIR-V) to a binary file.
    bool write_target_code_to_file(const char* path, slang::IBlob* code);

private:
    // Slang sessions used for compilation.
    Slang::ComPtr<slang::IGlobalSession> global_session_;
    Slang::ComPtr<slang::ISession> session_;

    static void append_diagnostics(std::string& dst, slang::IBlob* blob);
};