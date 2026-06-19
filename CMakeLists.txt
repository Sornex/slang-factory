cmake_minimum_required(VERSION 3.20)

project(SlangFactoryProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ---- Slang ----
# Adjust this path to your local Slang SDK location.
set(SLANG_DIR "D:/soft/slang")
set(SLANG_INCLUDE_DIR "${SLANG_DIR}/include")
set(SLANG_LIB_PATH "${SLANG_DIR}/lib/slang.lib")
set(SLANG_DLL_PATH "${SLANG_DIR}/bin/slang.dll")

# ---- Dependencies ----
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

function(copy_slang_dll target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SLANG_DLL_PATH}"
            $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

# ---- Library ----
set(SLANG_FACTORY_SOURCES
    src/slang_compiler.cpp
    src/slang_reflection_dumper.cpp
    src/slang_factory.cpp
    src/slang_shader_builder.cpp
    src/slang_reflection_extractor.cpp
)

add_library(slang_factory_lib ${SLANG_FACTORY_SOURCES})

target_include_directories(slang_factory_lib PUBLIC
    ${SLANG_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(slang_factory_lib PUBLIC
    "${SLANG_LIB_PATH}"
)

# ---- Wizard Demo ----
add_executable(slang_factory_demo
    demo/slang_factory_demo.cpp
)

target_link_libraries(slang_factory_demo PRIVATE
    slang_factory_lib
)

copy_slang_dll(slang_factory_demo)

# ---- Render Demo ----
add_executable(slang_factory_render_demo
    demo/render_demo/main.cpp
    demo/render_demo/vulkan_renderer.cpp
)

target_include_directories(slang_factory_render_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/render_demo
)

target_link_libraries(slang_factory_render_demo PRIVATE
    slang_factory_lib
    glfw
    Vulkan::Vulkan
)

copy_slang_dll(slang_factory_render_demo)