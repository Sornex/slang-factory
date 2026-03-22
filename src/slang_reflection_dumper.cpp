#include "helpers/slang_reflection_dumper.h"

SlangReflectionDumper::SlangReflectionDumper(std::ostream& o) : o (o) {}

// Just to make indents
void SlangReflectionDumper::indent(int n)
{
    for (int i = 0; i < n; ++i) o << "  ";
}

/*
So I have made this methode to get the name of resource shape I've tried to do it like in doc
void printResourceShape(SlangResourceShape shape)
{
    print("base shape:");
    switch(shape & SLANG_BASE_SHAPE_MASK)
    {
    case SLANG_TEXTURE1D: printf("TEXTURE1D"); break;
    case SLANG_TEXTURE2D: printf("TEXTURE2D"); break;
    // ...
    }

    if(shape & SLANG_TEXTURE_ARRAY_FLAG) printf("ARRAY");
    if(shape & SLANG_TEXTURE_MULTISAMPLE_FLAG) printf("MULTISAMPLE");
    // ...
}

but SLANG_TEXTURE1D, SLANG_BASE_SHAPE_MASK etc were undefined
*/

std::string SlangReflectionDumper::extract_resource_shape_name(const char* type_name)
{
    if (!type_name) return "<unknown>";

    std::string s(type_name);

    // So here I trim something like "Texture2D<float4>" into "Texture2D"
    const auto lt = s.find('<');
    if (lt != std::string::npos)
        s = s.substr(0, lt);

    // Also trimming all spaces just to be sure
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) ++start;

    if (start > 0) s = s.substr(start);

    return s.empty() ? "<unknown>" : s;
}

// Makes a string from stage enum
const char* SlangReflectionDumper::stage_to_string(SlangStage stage)
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

// Makes a string form Kind enum
const char* SlangReflectionDumper::type_kind_to_string(slang::TypeReflection::Kind k)
{
    using K = slang::TypeReflection::Kind;
    switch (k)
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

// makes a string from Sclar enum
const char* SlangReflectionDumper::scalar_type_to_string(slang::TypeReflection::ScalarType t)
{
    using ST = slang::TypeReflection::ScalarType;
    switch (t)
    {
    case ST::Void: return "void";
    case ST::Bool: return "bool";
    case ST::Int32: return "int32";
    case ST::UInt32: return "uint32";
    case ST::Int64: return "int64";
    case ST::UInt64: return "uint64";
    case ST::Float16: return "float16";
    case ST::Float32: return "float32";
    case ST::Float64: return "float64";
    default: return "unknown_scalar";
    }
}

// Makes a string from parameter category ebum
const char* SlangReflectionDumper::param_category_to_string(slang::ParameterCategory c)
{
    using PC = slang::ParameterCategory;
    switch (c)
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
    default: return "OtherCategory";
    }
}

// Makes a string from access enum
const char* SlangReflectionDumper::resource_access_to_string(SlangResourceAccess a)
{
    switch (a)
    {
    case SLANG_RESOURCE_ACCESS_NONE: return "NONE";
    case SLANG_RESOURCE_ACCESS_READ: return "READ";
    case SLANG_RESOURCE_ACCESS_READ_WRITE: return "READ_WRITE";
    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED: return "RASTER_ORDERED";
    case SLANG_RESOURCE_ACCESS_APPEND: return "APPEND";
    case SLANG_RESOURCE_ACCESS_CONSUME: return "CONSUME";
    default: return "UNKNOWN_ACCESS";
    }
}

// Prints the resource name as I've said made this a little bit akward
void SlangReflectionDumper::print_resource_shape(slang::TypeReflection* type)
{
    o << extract_resource_shape_name(type ? type->getName() : nullptr);
}

// Main Dump methode
void SlangReflectionDumper::dump(slang::ProgramLayout* program_layout)
{
    if (!program_layout)
    {
        o << "<null ProgramLayout>\n";
        return;
    }
    dump_program_layout(program_layout);
}

// Dumps the programm layout
void SlangReflectionDumper::dump_program_layout(slang::ProgramLayout* pl)
{
    o << "=== ProgramLayout ===\n";

    o << "global_scope:\n";
    dump_scope(pl->getGlobalParamsVarLayout(), 1);

    int ep_count = pl->getEntryPointCount();
    o << "entry_points(" << ep_count << "):\n";
    for (int i = 0; i < ep_count; ++i)
    {
        dump_entry_point(pl->getEntryPointByIndex(i), 1);
    }
}

// Dumps the global scope
void SlangReflectionDumper::dump_scope(slang::VariableLayoutReflection* scope_var_layout, int depth)
{
    if (!scope_var_layout)
    {
        indent(depth);
        o << "<null scope>\n";
        return;
    }

    slang::TypeLayoutReflection* tl = scope_var_layout->getTypeLayout();
    if (!tl)
    {
        indent(depth);
        o << "<scope has no type layout>\n";
        return;
    }

    if (tl->getKind() != slang::TypeReflection::Kind::Struct)
    {
        dump_var_layout(scope_var_layout, depth);
        return;
    }

    int field_count = tl->getFieldCount();
    indent(depth);
    o << "fields(" << field_count << "):\n";

    for (int i = 0; i < field_count; ++i)
    {
        auto field = tl->getFieldByIndex(i);
        indent(depth + 1);
        o << "-\n";
        dump_var_layout(field, depth + 2);
    }
}

// Dumps the entry point info
void SlangReflectionDumper::dump_entry_point(slang::EntryPointReflection* ep, int depth)
{
    if (!ep)
    {
        indent(depth);
        o << "<null entry point>\n";
        return;
    }

    indent(depth);
    o << "entry_point:\n";

    indent(depth + 1);
    o << "name: " << (ep->getName() ? ep->getName() : "<unnamed>") << "\n";

    indent(depth + 1);
    o << "stage: " << stage_to_string(ep->getStage()) << "\n";

    indent(depth + 1);
    o << "params_scope:\n";
    dump_scope(ep->getVarLayout(), depth + 2);

    auto result = ep->getResultVarLayout();
    if (result && result->getTypeLayout() &&
        result->getTypeLayout()->getKind() != slang::TypeReflection::Kind::None)
    {
        indent(depth + 1);
        o << "result:\n";
        dump_var_layout(result, depth + 2);
    }

    if (ep->getStage() == SLANG_STAGE_COMPUTE)
    {
        SlangUInt sizes[3] = { 0, 0, 0 };
        ep->getComputeThreadGroupSize(3, sizes);
        indent(depth + 1);
        o << "thread_group_size: "
             << sizes[0] << ", " << sizes[1] << ", " << sizes[2] << "\n";
    }
}

/* 
Prints layout offsets for a variable in all available parameter categories.
I iterate over VariableLayoutReflection categories (uniform bytes, resources, samplers, etc.)
and output the offset and optional binding space/set, so later I can understand how to bind it (If i ever need this).
*/
void SlangReflectionDumper::dump_var_offsets(slang::VariableLayoutReflection* var_layout, int depth)
{
    if (!var_layout) return;

    int cat_count = var_layout->getCategoryCount();
    if (cat_count <= 0) return;

    indent(depth);
    o << "offsets:\n";

    for (int i = 0; i < cat_count; ++i)
    {
        auto cat = var_layout->getCategoryByIndex(i);
        size_t off = var_layout->getOffset(cat);

        indent(depth + 1);
        o << "- " << param_category_to_string(cat)
             << " offset=" << off;

        size_t space = var_layout->getBindingSpace(cat);
        if (space != 0) o << " space=" << space;

        o << "\n";
    }
}

/* 
Dumps one reflected variable: its name, its binding/byte offsets, and then its type info.
I print both TypeLayout (how it is laid out / bound) and TypeReflection (what the type actually is).
*/
void SlangReflectionDumper::dump_var_layout(slang::VariableLayoutReflection* var_layout, int depth)
{
    if (!var_layout)
    {
        indent(depth);
        o << "<null var layout>\n";
        return;
    }

    indent(depth);
    o << "name: " << (var_layout->getName() ? var_layout->getName() : "<unnamed>") << "\n";

    dump_var_offsets(var_layout, depth + 1);

    slang::TypeLayoutReflection* tl = var_layout->getTypeLayout();
    indent(depth + 1);
    o << "type_layout:\n";
    dump_type_layout(tl, depth + 2);

    if (tl && tl->getType())
    {
        indent(depth + 1);
        o << "type:\n";
        dump_type(tl->getType(), depth + 2);
    }
}

/*
Prints the size of a type in each layout category.
I use TypeLayoutReflection here because size depends on how the type is laid out for the target.
*/
void SlangReflectionDumper::dump_type_sizes(slang::TypeLayoutReflection* type_layout, int depth)
{
    if (!type_layout) return;

    int cat_count = type_layout->getCategoryCount();
    if (cat_count <= 0) return;

    indent(depth);
    o << "sizes:\n";

    for (int i = 0; i < cat_count; ++i)
    {
        auto cat = type_layout->getCategoryByIndex(i);
        size_t sz = type_layout->getSize(cat);

        indent(depth + 1);
        o << "- " << param_category_to_string(cat) << " size=" << sz << "\n";
    }
}

/*
Dumps detailed layout info for a type (TypeLayoutReflection): name, kind, sizes, alignment and stride.
Then I recursively go deeper depending on the kind: struct fields, array/vector/matrix element layout,
and for containers (cbuffer/parameter block) I print both container and element layouts.
*/
void SlangReflectionDumper::dump_type_layout(slang::TypeLayoutReflection* type_layout, int depth)
{
    if (!type_layout)
    {
        indent(depth);
        o << "<null type layout>\n";
        return;
    }

    const char* name = type_layout->getName();
    auto kind = type_layout->getKind();

    indent(depth);
    o << "name: " << (name ? name : "<anon>") << "\n";

    indent(depth);
    o << "kind: " << type_kind_to_string(kind) << "\n";

    dump_type_sizes(type_layout, depth);

    indent(depth);
    o << "alignment(bytes): " << type_layout->getAlignment() << "\n";
    indent(depth);
    o << "stride(bytes): " << type_layout->getStride() << "\n";

    switch (kind)
    {
    case slang::TypeReflection::Kind::Struct:
    {
        int field_count = type_layout->getFieldCount();
        indent(depth);
        o << "fields(" << field_count << "):\n";
        for (int f = 0; f < field_count; ++f)
        {
            auto field = type_layout->getFieldByIndex(f);
            indent(depth + 1);
            o << "-\n";
            dump_var_layout(field, depth + 2);
        }
        break;
    }

    case slang::TypeReflection::Kind::Array:
        indent(depth);
        o << "element_count: " << type_layout->getElementCount() << "\n";
        indent(depth);
        o << "element_type_layout:\n";
        dump_type_layout(type_layout->getElementTypeLayout(), depth + 1);
        break;

    case slang::TypeReflection::Kind::Vector:
        indent(depth);
        o << "element_count: " << type_layout->getElementCount() << "\n";
        indent(depth);
        o << "element_type_layout:\n";
        dump_type_layout(type_layout->getElementTypeLayout(), depth + 1);
        break;

    case slang::TypeReflection::Kind::Matrix:
        indent(depth);
        o << "row_count: " << type_layout->getRowCount() << "\n";
        indent(depth);
        o << "column_count: " << type_layout->getColumnCount() << "\n";
        indent(depth);
        o << "element_type_layout:\n";
        dump_type_layout(type_layout->getElementTypeLayout(), depth + 1);
        break;

    case slang::TypeReflection::Kind::ConstantBuffer:
    case slang::TypeReflection::Kind::ParameterBlock:
    case slang::TypeReflection::Kind::TextureBuffer:
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
    {
        indent(depth);
        o << "container_var_layout:\n";
        dump_var_offsets(type_layout->getContainerVarLayout(), depth + 1);

        indent(depth);
        o << "element_var_layout:\n";
        auto elem = type_layout->getElementVarLayout();
        dump_var_offsets(elem, depth + 1);

        if (elem && elem->getTypeLayout())
        {
            indent(depth);
            o << "element_type_layout:\n";
            dump_type_layout(elem->getTypeLayout(), depth + 1);
        }
        break;
    }

    default:
        break;
    }
}

/*
Dumps pure type information (TypeReflection) without layout/binding details.
I use this to understand what the type actually is (scalar/vector/struct/resource, etc.)
and then recursively print element/field/result types for arrays, structs, resources, and containers.
*/
void SlangReflectionDumper::dump_type(slang::TypeReflection* type, int depth)
{
    if (!type)
    {
        indent(depth);
        o << "<null type>\n";
        return;
    }

    auto kind = type->getKind();
    const char* name = type->getName();

    indent(depth);
    o << "name: " << (name ? name : "<anon>") << "\n";

    indent(depth);
    o << "kind: " << type_kind_to_string(kind) << "\n";

    switch (kind)
    {
    case slang::TypeReflection::Kind::Scalar:
        indent(depth);
        o << "scalar: " << scalar_type_to_string(type->getScalarType()) << "\n";
        break;

    case slang::TypeReflection::Kind::Resource:
        indent(depth);
        o << "resource_shape: ";
        print_resource_shape(type);
        o << "\n";

        indent(depth);
        o << "resource_access: " << resource_access_to_string(type->getResourceAccess()) << "\n";

        indent(depth);
        o << "resource_result_type:\n";
        dump_type(type->getResourceResultType(), depth + 1);
        break;

    case slang::TypeReflection::Kind::Struct:
    {
        int field_count = type->getFieldCount();
        indent(depth);
        o << "fields(" << field_count << "):\n";

        for (int i = 0; i < field_count; ++i)
        {
            auto f = type->getFieldByIndex(i);

            indent(depth + 1);
            o << "- name: " << (f && f->getName() ? f->getName() : "<unnamed>") << "\n";

            if (f && f->getType())
            {
                indent(depth + 2);
                o << "field_type:\n";
                dump_type(f->getType(), depth + 3);
            }
        }
        break;
    }

    case slang::TypeReflection::Kind::Array:
    case slang::TypeReflection::Kind::Vector:
        indent(depth);
        o << "element_count: " << type->getElementCount() << "\n";
        indent(depth);
        o << "element_type:\n";
        dump_type(type->getElementType(), depth + 1);
        break;

    case slang::TypeReflection::Kind::Matrix:
        indent(depth);
        o << "row_count: " << type->getRowCount() << "\n";
        indent(depth);
        o << "column_count: " << type->getColumnCount() << "\n";
        indent(depth);
        o << "element_type:\n";
        dump_type(type->getElementType(), depth + 1);
        break;

    case slang::TypeReflection::Kind::ConstantBuffer:
    case slang::TypeReflection::Kind::ParameterBlock:
    case slang::TypeReflection::Kind::TextureBuffer:
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        indent(depth);
        o << "element_type:\n";
        dump_type(type->getElementType(), depth + 1);
        break;

    default:
        break;
    }
}