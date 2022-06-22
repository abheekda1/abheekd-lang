cmake_minimum_required(VERSION 3.21)
project(abheek_lang)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
        src/Lexer/Lexer.cpp
        src/Token/Token.cpp
        src/AST/AST.cpp
        src/Parser/Parser.cpp)

add_executable(abheek_lang ${SOURCE} main.cpp)

target_include_directories(abheek_lang PUBLIC include)