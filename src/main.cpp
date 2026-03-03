#include <iostream>
#include <slang.h>

int main()
{
    std::cout << "Slang Factory Project started.\n";

    const char* version = spGetBuildTagString();
    if (version)
    {
        std::cout << "Slang build tag: " << version << '\n';
    }
    else
    {
        std::cout << "Could not read Slang build tag.\n";
    }

    return 0;
}