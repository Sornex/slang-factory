#pragma once
#include <string>
#include "helpers/shader_request.h"

// Generates Slang source code for a given ShaderRequest (for now only vertex or fragment).
class SlangFactory
{
public:
    // Returns full Slang code (as a single module) containing one entry point.
    std::string generate(const ShaderRequest& req) const;

private:
    std::string generate_vertex_shader(const ShaderRequest& req) const;
    std::string generate_fragment_shader(const ShaderRequest& req) const;
};