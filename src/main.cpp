#include <iostream>
#include <string>

#include "helpers/slang_compiler.h"
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
    SlangCompiler compiler;
    if (!compiler.init_spirv_1_5())
    {
        std::cerr << "Failed to initialize SlangCompiler.\n";
        return -1;
    }

    auto result = compiler.compile_from_source_string(
        "generated_module",
        "generated_module.slang",
        kSlangSource,
        "computeMain");

    if (result.diagnostics)
        print_diagnostics(result.diagnostics.get());

    if (!result.layout)
    {
        std::cerr << "Compilation succeeded but no ProgramLayout was returned.\n";
        return -1;
    }

    SlangReflectionDumper dumper(std::cout);
    dumper.dump(result.layout);

    return 0;
}