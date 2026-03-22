cmake_minimum_required(VERSION 3.20)

project(SlangFactoryProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Slang path ----
set(SLANG_DIR "D:/soft/slang")
set(SLANG_INCLUDE_DIR "${SLANG_DIR}/include")

set(SLANG_LIB_PATH "${SLANG_DIR}/lib/slang.lib")
set(SLANG_DLL_PATH "${SLANG_DIR}/bin/slang.dll")

add_executable(slang_factory
    src/main.cpp
    src/slang_factory.cpp
    src/slang_reflection_dumper.cpp
    src/slang_compiler.cpp
)

target_include_directories(slang_factory PRIVATE
    ${SLANG_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(slang_factory PRIVATE
    "${SLANG_LIB_PATH}"
)

add_custom_command(TARGET slang_factory POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SLANG_DLL_PATH}"
        $<TARGET_FILE_DIR:slang_factory>
)