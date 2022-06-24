cmake_minimum_required(VERSION 3.21)
project(abheek_lang)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
        src/Lexer/Lexer.cpp
        src/Token/Token.cpp
        src/AST/AST.cpp
        src/Parser/Parser.cpp)

message(STATUS "${LLVM_INCLUDE_DIR}")

add_subdirectory(vendor/llvm-project/llvm)

include_directories(
        include
        "${LLVM_SOURCE_DIR}/include"
        "${LLVM_BINARY_DIR}/include"
)

add_executable(abheek_lang ${SOURCE} main.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(abheek_lang ${llvm_libs})