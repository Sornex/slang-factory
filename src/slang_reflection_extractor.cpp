#include "helpers/slang_reflection_extractor.h"

#include <sstream>

std::string SlangReflectionExtractor::stage_to_string(SlangStage stage) const
{
    switch (stage)
    {
    case SLANG_STAGE_NONE: return "NONE";
    case SLANG_STAGE_VERTEX: return "VERTEX";
    case SLANG_STAGE_HULL: return "HULL";
    case SLANG_STAGE_DOMAIN: return "DOMAIN";
    case SLANG_STAGE_GEOMETRY: return "GEOMETRY";
    case SLANG_STAGE_FRAGMENT: return "FRAGMENT";
    case SLANG_STAGE_COMPUTE: return "COMPUTE";
    default: return "UNKNOWN_STAGE";
    }
}

std::string SlangReflectionExtractor::type_kind_to_string(slang::TypeReflection::Kind kind) const
{
    using K = slang::TypeReflection::Kind;

    switch (kind)
    {
    case K::None: return "None";
    case K::Scalar: return "Scalar";
    case K::Vector: return "Vector";
    case K::Matrix: return "Matrix";
    case K::Array: return "Array";
    case K::Struct: return "Struct";
    case K::Resource: return "Resource";
    case K::SamplerState: return "SamplerState";
    case K::ConstantBuffer: return "ConstantBuffer";
    case K::ParameterBlock: return "ParameterBlock";
    case K::TextureBuffer: return "TextureBuffer";
    case K::ShaderStorageBuffer: return "ShaderStorageBuffer";
    default: return "Other";
    }
}

std::string SlangReflectionExtractor::param_category_to_string(slang::ParameterCategory category) const
{
    using PC = slang::ParameterCategory;

    switch (category)
    {
    case PC::None: return "None";
    case PC::Uniform: return "Uniform(Bytes)";
    case PC::ConstantBuffer: return "ConstantBuffer";
    case PC::ShaderResource: return "ShaderResource(t)";
    case PC::UnorderedAccess: return "UnorderedAccess(u)";
    case PC::SamplerState: return "SamplerState(s)";
    case PC::VaryingInput: return "VaryingInput";
    case PC::VaryingOutput: return "VaryingOutput";
    case PC::SubElementRegisterSpace: return "SubElementRegisterSpace(space/set)";
    default:
    {
        std::ostringstream oss;
        oss << "OtherCategory(" << static_cast<int>(category) << ")";
        return oss.str();
    }
    }
}

ReflectedField SlangReflectionExtractor::extract_var_layout(slang::VariableLayoutReflection* var_layout)
{
    ReflectedField field;

    if (!var_layout)
    {
        field.name = "<null>";
        field.type_name = "<null>";
        field.kind = "Null";
        return field;
    }

    field.name = var_layout->getName() ? var_layout->getName() : "<unnamed>";

    const int cat_count = var_layout->getCategoryCount();
    for (int i = 0; i < cat_count; ++i)
    {
        auto category = var_layout->getCategoryByIndex(i);

        ReflectedOffset off;
        off.category = param_category_to_string(category);
        off.offset = var_layout->getOffset(category);
        off.space = var_layout->getBindingSpace(category);

        field.offsets.push_back(off);
    }

    auto* type_layout = var_layout->getTypeLayout();
    if (type_layout)
    {
        ReflectedField type_info = extract_type_layout(type_layout, field.name);
        field.type_name = type_info.type_name;
        field.kind = type_info.kind;
        field.children = std::move(type_info.children);
    }
    else
    {
        field.type_name = "<null>";
        field.kind = "Null";
    }

    return field;
}

ReflectedField SlangReflectionExtractor::extract_type_layout(
    slang::TypeLayoutReflection* type_layout,
    const std::string& fallback_name)
{
    ReflectedField field;

    if (!type_layout)
    {
        field.name = fallback_name;
        field.type_name = "<null>";
        field.kind = "Null";
        return field;
    }

    field.name = fallback_name;
    field.type_name = type_layout->getName() ? type_layout->getName() : "<anon>";
    field.kind = type_kind_to_string(type_layout->getKind());

    using Kind = slang::TypeReflection::Kind;

    switch (type_layout->getKind())
    {
    case Kind::Struct:
    {
        const int field_count = type_layout->getFieldCount();
        for (int i = 0; i < field_count; ++i)
        {
            auto* child = type_layout->getFieldByIndex(i);
            field.children.push_back(extract_var_layout(child));
        }
        break;
    }

    case Kind::ConstantBuffer:
    case Kind::ParameterBlock:
    case Kind::TextureBuffer:
    case Kind::ShaderStorageBuffer:
    {
        // Prefer flattening the element struct directly, so engine code sees actual buffer fields
        auto* elem = type_layout->getElementVarLayout();
        if (elem && elem->getTypeLayout())
        {
            auto* elem_layout = elem->getTypeLayout();
            if (elem_layout->getKind() == Kind::Struct)
            {
                const int field_count = elem_layout->getFieldCount();
                for (int i = 0; i < field_count; ++i)
                {
                    auto* child = elem_layout->getFieldByIndex(i);
                    field.children.push_back(extract_var_layout(child));
                }
            }
            else
            {
                field.children.push_back(extract_var_layout(elem));
            }
        }
        break;
    }

    case Kind::Array:
    case Kind::Vector:
    case Kind::Matrix:
    {
        auto* elem_layout = type_layout->getElementTypeLayout();
        if (elem_layout)
        {
            ReflectedField elem_field = extract_type_layout(elem_layout, "element");
            field.children.push_back(std::move(elem_field));
        }
        break;
    }

    default:
        break;
    }

    return field;
}

std::vector<ReflectedField> SlangReflectionExtractor::extract_scope_fields(slang::VariableLayoutReflection* scope_var_layout)
{
    std::vector<ReflectedField> result;

    if (!scope_var_layout)
        return result;

    auto* type_layout = scope_var_layout->getTypeLayout();
    if (!type_layout)
        return result;

    if (type_layout->getKind() != slang::TypeReflection::Kind::Struct)
    {
        result.push_back(extract_var_layout(scope_var_layout));
        return result;
    }

    const int field_count = type_layout->getFieldCount();
    for (int i = 0; i < field_count; ++i)
    {
        auto* field = type_layout->getFieldByIndex(i);
        result.push_back(extract_var_layout(field));
    }

    return result;
}

std::vector<ReflectedField> SlangReflectionExtractor::extract_result_fields(slang::VariableLayoutReflection* result_var_layout)
{
    std::vector<ReflectedField> result;

    if (!result_var_layout)
        return result;

    result.push_back(extract_var_layout(result_var_layout));
    return result;
}

ReflectedProgram SlangReflectionExtractor::extract(slang::ProgramLayout* layout)
{
    ReflectedProgram program;

    if (!layout)
        return program;

    program.global_fields = extract_scope_fields(layout->getGlobalParamsVarLayout());

    const int ep_count = layout->getEntryPointCount();
    for (int i = 0; i < ep_count; ++i)
    {
        auto* ep = layout->getEntryPointByIndex(i);
        if (!ep)
            continue;

        ReflectedStage stage;
        stage.entry_name = ep->getName() ? ep->getName() : "<unnamed>";
        stage.stage = stage_to_string(ep->getStage());
        stage.parameters = extract_scope_fields(ep->getVarLayout());
        stage.result_fields = extract_result_fields(ep->getResultVarLayout());

        program.entry_points.push_back(std::move(stage));
    }

    return program;
}

const ReflectedField* SlangReflectionExtractor::find_field_by_name(
    const std::vector<ReflectedField>& fields,
    const std::string& name)
{
    for (const auto& field : fields)
    {
        if (field.name == name)
            return &field;
    }
    return nullptr;
}

const ReflectedField* SlangReflectionExtractor::find_field_by_path(
    const std::vector<ReflectedField>& fields,
    const std::vector<std::string>& path)
{
    if (path.empty())
        return nullptr;

    const ReflectedField* current = find_field_by_name(fields, path[0]);
    if (!current)
        return nullptr;

    for (size_t i = 1; i < path.size(); ++i)
    {
        current = find_field_by_name(current->children, path[i]);
        if (!current)
            return nullptr;
    }

    return current;
}

void SlangReflectionExtractor::collect_fields_by_kind(
    const std::vector<ReflectedField>& fields,
    const std::string& kind,
    std::vector<const ReflectedField*>& out_fields)
{
    for (const auto& field : fields)
    {
        if (field.kind == kind)
            out_fields.push_back(&field);

        collect_fields_by_kind(field.children, kind, out_fields);
    }
}

void SlangReflectionExtractor::flatten_uniform_fields(const std::vector<ReflectedField>& fields, std::vector<const ReflectedField*>& out_fields)
{
    for (const auto& field : fields)
    {
        const bool is_real_named_field =
            !field.name.empty() &&
            field.name != "<unnamed>" &&
            field.name != "element";

        const bool is_value_field =
            field.kind == "Scalar" ||
            field.kind == "Vector" ||
            field.kind == "Matrix" ||
            field.kind == "Array";

        if (is_real_named_field && is_value_field)
        {
            out_fields.push_back(&field);
        }

        flatten_uniform_fields(field.children, out_fields);
    }
}

std::vector<const ReflectedField*> SlangReflectionExtractor::get_resource_fields(const ReflectedProgram& program)
{
    std::vector<const ReflectedField*> out_fields;
    collect_fields_by_kind(program.global_fields, "Resource", out_fields);
    collect_fields_by_kind(program.global_fields, "SamplerState", out_fields);
    return out_fields;
}

std::vector<const ReflectedField*> SlangReflectionExtractor::get_uniform_leaf_fields(const ReflectedProgram& program)
{
    std::vector<const ReflectedField*> out_fields;
    flatten_uniform_fields(program.global_fields, out_fields);
    return out_fields;
}

std::vector<const ReflectedField*> SlangReflectionExtractor::get_stage_parameter_fields(const ReflectedStage& stage)
{
    std::vector<const ReflectedField*> out_fields;

    for (const auto& param : stage.parameters)
    {
        if (!param.children.empty())
        {
            for (const auto& child : param.children)
            {
                out_fields.push_back(&child);
            }
        }
        else
        {
            out_fields.push_back(&param);
        }
    }

    return out_fields;
}

std::vector<const ReflectedField*> SlangReflectionExtractor::get_stage_result_fields(const ReflectedStage& stage)
{
    std::vector<const ReflectedField*> out_fields;

    for (const auto& result : stage.result_fields)
    {
        if (!result.children.empty())
        {
            for (const auto& child : result.children)
            {
                out_fields.push_back(&child);
            }
        }
        else
        {
            out_fields.push_back(&result);
        }
    }

    return out_fields;
}