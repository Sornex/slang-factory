cmake_minimum_required(VERSION 3.20)

project(SlangFactoryProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Slang path ----
set(SLANG_DIR "D:/soft/slang")
set(SLANG_INCLUDE_DIR "${SLANG_DIR}/include")

set(SLANG_LIB_PATH "${SLANG_DIR}/lib/slang.lib")
set(SLANG_DLL_PATH "${SLANG_DIR}/bin/slang.dll")

# --- Library ----

add_library(slang_factory_lib
    src/slang_compiler.cpp
    src/slang_reflection_dumper.cpp
    src/slang_factory.cpp
    src/slang_shader_builder.cpp
    src/slang_reflection_extractor.cpp
)

target_include_directories(slang_factory_lib PUBLIC
    ${SLANG_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(slang_factory_lib PUBLIC
    "${SLANG_LIB_PATH}"
)


# ---- Demo ----
add_executable(slang_factory_demo
    demo/slang_factory_demo.cpp
)

target_link_libraries(slang_factory_demo PRIVATE
    slang_factory_lib
)

add_custom_command(TARGET slang_factory_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SLANG_DLL_PATH}"
        $<TARGET_FILE_DIR:slang_factory_demo>
)