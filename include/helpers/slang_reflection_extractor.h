#pragma once

#include <string>
#include <vector>

#include <slang.h>

// One reflected offset entry for a specific parameter category.
struct ReflectedOffset
{
    std::string category;
    size_t offset = 0;
    size_t space = 0;
};


// A reflected field with optional child fields for nested structures.
struct ReflectedField
{
    std::string name;
    std::string type_name;
    std::string kind;

    std::vector<ReflectedOffset> offsets;
    std::vector<ReflectedField> children;
};

// Reflection data for one shader entry point.
struct ReflectedStage
{
    std::string entry_name;
    std::string stage;

    std::vector<ReflectedField> parameters;
    std::vector<ReflectedField> result_fields;
};

// Top-level extracted reflection for a compiled shader program.
struct ReflectedProgram
{
    std::vector<ReflectedField> global_fields;
    std::vector<ReflectedStage> entry_points;
};

class SlangReflectionExtractor
{
public:
    ReflectedProgram extract(slang::ProgramLayout* layout);

    // Helpers for engine-style access

    // Query helpers
    static const ReflectedField* find_field_by_name(const std::vector<ReflectedField>& fields, const std::string& name);
    static const ReflectedField* find_field_by_path(const std::vector<ReflectedField>& fields, const std::vector<std::string>& path);

    // Collection helpers
    static void collect_fields_by_kind(const std::vector<ReflectedField>& fields, const std::string& kind, std::vector<const ReflectedField*>& out_fields);
    static void flatten_uniform_fields(const std::vector<ReflectedField>& fields, std::vector<const ReflectedField*>& out_fields);

    // Convenience helpers
    static std::vector<const ReflectedField*> get_resource_fields(const ReflectedProgram& program);
    static std::vector<const ReflectedField*> get_uniform_leaf_fields(const ReflectedProgram& program);
    static std::vector<const ReflectedField*> get_stage_parameter_fields(const ReflectedStage& stage);
    static std::vector<const ReflectedField*> get_stage_result_fields(const ReflectedStage& stage);

private:
    ReflectedField extract_var_layout(slang::VariableLayoutReflection* var_layout);
    ReflectedField extract_type_layout(slang::TypeLayoutReflection* type_layout, const std::string& fallback_name = "");

    std::vector<ReflectedField> extract_scope_fields(slang::VariableLayoutReflection* scope_var_layout);
    std::vector<ReflectedField> extract_result_fields(slang::VariableLayoutReflection* result_var_layout);

    std::string stage_to_string(SlangStage stage) const;
    std::string type_kind_to_string(slang::TypeReflection::Kind kind) const;
    std::string param_category_to_string(slang::ParameterCategory category) const;
};