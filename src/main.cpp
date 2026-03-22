#include <iostream>
#include <string>

#include <slang.h>
#include <slang-com-ptr.h>

#include "helpers/slang_reflection_dumper.h"

// For now I'm taking just a constant string with slang code, in future I will generate it myself, hopefully))
static const char* kSlangSource = R"(
cbuffer Params
{
    float4 color;
    int mode;
};

Texture2D<float4> input_tex;
SamplerState samp;
RWTexture2D<float4> output_tex;

[numthreads(8, 8, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = (float2(tid.xy) + 0.5) / 512.0;
    float4 v = input_tex.SampleLevel(samp, uv, 0);
    output_tex[tid.xy] = (mode == 0) ? v : color;
}
)";

// Made it for a debug on all stages
static void print_diagnostics(slang::IBlob* diagnostics)
{
    if (!diagnostics) return;
    const char* text = (const char*)diagnostics->getBufferPointer();
    if (text && text[0]) std::cerr << text << "\n";
}

int main()
{
    using Slang::ComPtr;

    // I was following a Using the Compilation API tutorial here https://docs.shader-slang.org/en/latest/compilation-api.html
    // 1) Create Global Session
    ComPtr<slang::IGlobalSession> global_session;
    if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef())))
    {
        std::cerr << "Failed to create global session.\n";
        return -1;
    }

    // 2) Create Session
    slang::SessionDesc session_desc = {};

    // List of enabled compilation targets, so in this step I selected spir 5, it was given also in example, but I guess I will really use spir 5, because it's what Vulkan uses
    slang::TargetDesc target_desc = {};
    target_desc.format = SLANG_SPIRV;
    target_desc.profile = global_session->findProfile("spirv_1_5");

    session_desc.targets = &target_desc;
    session_desc.targetCount = 1;

    // Create the session
    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(global_session->createSession(session_desc, session.writeRef())))
    {
        std::cerr << "Failed to create Slang session.\n";
        return -1;
    }

    // 3) Load Modules
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;

    // Here I've obviously used my string as a source
    module = session->loadModuleFromSourceString(
        "generated_module",          // moduleName
        "generated_module.slang",     // modulePath
        kSlangSource,                // source
        diagnostics.writeRef()        // diagnostics
    );

    if (!module)
    {
        print_diagnostics(diagnostics.get());
        std::cerr << "Failed to load module from source string.\n";
        return -1;
    }

    // 4) Query Entry Points
    ComPtr<slang::IEntryPoint> entry_point;
    diagnostics.setNull();
    
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entry_point.writeRef())))
    {
        print_diagnostics(diagnostics.get());
        std::cerr << "Failed to find entry point 'computeMain'.\n";
        return -1;
    }

    // 5) Compose Modules and Entry Points, I've changed this part slightly bcs I was getting errors with an example in tutorial
    slang::IComponentType* components[] = { module.get(), entry_point.get() };
    ComPtr<slang::IComponentType> program;
    diagnostics.setNull();

    if (SLANG_FAILED(session->createCompositeComponentType(
            components,
            2,
            program.writeRef(),
            diagnostics.writeRef())))
    {
        print_diagnostics(diagnostics.get());
        std::cerr << "Failed to create composite component type.\n";
        return -1;
    }


    // 6) Link
    Slang::ComPtr<slang::IComponentType> linked_program;
    diagnostics.setNull();

    if (SLANG_FAILED(program->link(linked_program.writeRef(), diagnostics.writeRef())))
    {
        print_diagnostics(diagnostics.get());
        std::cerr << "Failed to link program.\n";
        return -1;
    }

    // 7) Reflection

    slang::ProgramLayout* layout = linked_program->getLayout();
    if (!layout)
    {
        std::cerr << "No ProgramLayout returned (reflection unavailable).\n";
        return -1;
    }

    SlangReflectionDumper dumper(std::cout);
    dumper.dump(layout);

    return 0;
}