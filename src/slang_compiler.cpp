#include "helpers/slang_compiler.h"

#include <iostream>

void SlangCompiler::append_diagnostics(std::string& dst, slang::IBlob* blob)
{
    if (!blob) return;
    const char* text = (const char*)blob->getBufferPointer();
    if (!text || !text[0]) return;

    if (!dst.empty()) dst += "\n";
    dst += text;
}

bool SlangCompiler::init_spirv_1_5()
{
    if (global_session_ && session_)
        return true;

    // Create Global Session
    if (SLANG_FAILED(slang::createGlobalSession(global_session_.writeRef())))
        return false;

    // Create Session
    slang::SessionDesc session_desc = {};

    slang::TargetDesc target_desc = {};
    target_desc.format = SLANG_SPIRV;
    target_desc.profile = global_session_->findProfile("spirv_1_5");

    session_desc.targets = &target_desc;
    session_desc.targetCount = 1;

    if (SLANG_FAILED(global_session_->createSession(session_desc, session_.writeRef())))
        return false;

    return true;
}

SlangCompiler::Result SlangCompiler::compile_from_source_string(
    const char* module_name,
    const char* virtual_path,
    const char* source,
    const char* entry_point_name)
{
    Result r;

    if (!session_)
    {
        // Not initialized
        return r;
    }

    // Load Modules
    Slang::ComPtr<slang::IModule> module;
    module = session_->loadModuleFromSourceString(
        module_name,
        virtual_path,
        source,
        r.diagnostics.writeRef());

    if (!module)
        return r;

    // Query Entry Points
    Slang::ComPtr<slang::IEntryPoint> entry_point;
    if (SLANG_FAILED(module->findEntryPointByName(entry_point_name, entry_point.writeRef())))
        return r;
    

    // Compose Modules and Entry Points, I've changed this part slightly bcs I was getting errors with an example in tutorial
    slang::IComponentType* components[] = { module.get(), entry_point.get() };

    Slang::ComPtr<slang::IComponentType> program;
    r.diagnostics.setNull();

    if (SLANG_FAILED(session_->createCompositeComponentType(
            components,
            2,
            program.writeRef(),
            r.diagnostics.writeRef())))
        return r;

    // Link
    r.diagnostics.setNull();
    if (SLANG_FAILED(program->link(r.linked_program.writeRef(), r.diagnostics.writeRef())))
        return r;

    // Layout is owned by linked_program, so keep linked_program alive in Result.
    r.layout = r.linked_program ? r.linked_program->getLayout() : nullptr;
    return r;
}