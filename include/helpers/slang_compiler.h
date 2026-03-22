#pragma once

#include <string>

#include <slang.h>
#include <slang-com-ptr.h>

// I was following a Using the Compilation API tutorial here https://docs.shader-slang.org/en/latest/compilation-api.html

class SlangCompiler
{
public:
    struct Result
    {
        Slang::ComPtr<slang::IComponentType> linked_program;
        slang::ProgramLayout* layout = nullptr;

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

private:
    // Basicaly just helpers
    Slang::ComPtr<slang::IGlobalSession> global_session_;
    Slang::ComPtr<slang::ISession> session_;

    static void append_diagnostics(std::string& dst, slang::IBlob* blob);
};