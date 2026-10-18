cmake_minimum_required(VERSION 3.18)
project(zlua_luau_editor LANGUAGES C CXX)

add_library(luau SHARED luau_editor_dll.cpp luau_exports.c)
target_include_directories(luau PRIVATE
    "${LUAU_SRC}/VM/include"
    "${LUAU_SRC}/Compiler/include")
target_compile_features(luau PRIVATE cxx_std_17)
target_compile_options(luau PRIVATE /EHsc)
if(MSVC)
    set_property(TARGET luau PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()
target_link_libraries(luau PRIVATE
    "${LUAU_LIB_DIR}/Luau.Compiler.lib"
    "${LUAU_LIB_DIR}/Luau.Ast.lib"
    "${LUAU_LIB_DIR}/Luau.Bytecode.lib"
    "${LUAU_LIB_DIR}/Luau.Inliner.lib"
    "${LUAU_LIB_DIR}/Luau.VM.lib"
    "${LUAU_LIB_DIR}/Luau.Common.lib")
set_target_properties(luau PROPERTIES OUTPUT_NAME "luau")
