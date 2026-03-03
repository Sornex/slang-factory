#include "slang_factory/slang_factory.h"

std::string SlangFactory::generate_shader_code() const
{
    return R"(
[numthreads(1, 1, 1)]
void computeMain()
{
}
)";
}