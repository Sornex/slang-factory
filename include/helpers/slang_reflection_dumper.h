#pragma once

#include <slang.h>
#include <ostream>
#include <iostream>

/*
So here is my dumper. For the hieararchy i've decided to dump it like this

=== ProgramLayout ===
- Global Scope
    - fields (name, type, etc)
- Entry Points
    - entry point (name, stage, scope)
        -scope

*/

class SlangReflectionDumper
{
public:
    explicit SlangReflectionDumper(std::ostream& o = std::cout);

    void dump(slang::ProgramLayout* program_layout);

private:
    std::ostream& o;

    // Main dumps
    void dump_program_layout(slang::ProgramLayout* pl);
    void dump_scope(slang::VariableLayoutReflection* scope_var_layout, int depth);
    void dump_entry_point(slang::EntryPointReflection* ep, int depth);

    // Variable + type layout
    void dump_var_layout(slang::VariableLayoutReflection* var_layout, int depth);
    void dump_var_offsets(slang::VariableLayoutReflection* var_layout, int depth);

    void dump_type_layout(slang::TypeLayoutReflection* type_layout, int depth);
    void dump_type_sizes(slang::TypeLayoutReflection* type_layout, int depth);

    // TypeReflection (no layout)
    void dump_type(slang::TypeReflection* type, int depth);

    // Helpers
    void indent(int n);
    const char* stage_to_string(SlangStage stage);
    const char* type_kind_to_string(slang::TypeReflection::Kind k);
    const char* scalar_type_to_string(slang::TypeReflection::ScalarType t);
    const char* param_category_to_string(slang::ParameterCategory c);
    const char* resource_access_to_string(SlangResourceAccess a);

    // Maybe will need to fix this
    std::string extract_resource_shape_name(const char* type_name);
    void print_resource_shape(slang::TypeReflection* type);
};