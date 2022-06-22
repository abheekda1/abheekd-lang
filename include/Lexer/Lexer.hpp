//
// Created by abheekd on 6/21/2022.
//

#ifndef ABHEEK_LANG_LEXER_HPP
#define ABHEEK_LANG_LEXER_HPP

#include <string>
#include <vector>

#include "Token/Token.hpp"

class Lexer {
public:
    Lexer();
    Lexer(std::string source);

    ~Lexer();

    static std::string Source;

    static Token getTok();
    static char LastChar;
    static int CharIdx;

    static position Position;

    static std::vector<Token> Tokens;
};


#endif //ABHEEK_LANG_LEXER_HPP
