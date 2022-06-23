cmake_minimum_required(VERSION 3.21)
project(abheek_lang)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
        src/Lexer/Lexer.cpp
        src/Token/Token.cpp
        src/AST/AST.cpp
        src/Parser/Parser.cpp)

set(LLVM_DIR "/opt/homebrew/Cellar/llvm/13.0.1_1/lib/cmake/llvm")
message(${LLVM_DIR})

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(abheek_lang ${SOURCE} main.cpp)

target_include_directories(abheek_lang PUBLIC
        include
        ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(abheek_lang PUBLIC
        ${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(abheek_lang ${llvm_libs})