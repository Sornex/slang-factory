#pragma once
#include <string>
#include "helpers/shader_request.h"

// Generates Slang source code for a given ShaderRequest (for now only vertex or fragment).
class SlangFactory
{
public:
    // Generates one Slang module that can contain VS, FS, or both.
    std::string generate_pipeline_module(const ShaderRequest& req) const;

private:
    void emit_common_types(const ShaderRequest& req, std::string& out) const;
    void emit_vertex_entry(const ShaderRequest& req, std::string& out) const;
    void emit_fragment_entry(const ShaderRequest& req, std::string& out) const;
};