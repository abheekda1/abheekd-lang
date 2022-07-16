//
// Created by abheekd on 6/21/2022.
//

#include <limits>
#include <string>
#include <utility>

#include "Token/Token.hpp"

std::map<std::string, int> Token::BinOpPrecedence;

Token::Token() : type(type::tok_other) {}
Token::Token(enum type type) : type(type) {}
Token::Token(enum type type, std::string value, struct position position) :
        type(type), value(std::move(value)), pos(position) {}

void Token::InitBinOps() {
    Token::BinOpPrecedence["*"] = 5;
    Token::BinOpPrecedence["/"] = 5;
    Token::BinOpPrecedence["%"] = 5;

    Token::BinOpPrecedence["+"] = 6;
    Token::BinOpPrecedence["-"] = 6;

    Token::BinOpPrecedence["<<"] = 7;
    Token::BinOpPrecedence[">>"] = 7;

    Token::BinOpPrecedence["<"] = 9;
    Token::BinOpPrecedence["<="] = 9;
    Token::BinOpPrecedence[">"] = 9;
    Token::BinOpPrecedence[">="] = 79;
}

int Token::GetPrecedence() {
    for (const auto &C : value)
        if (!isascii(C))
            return std::numeric_limits<int>::max();

    // make sure it has been declared
    int TokenPrecedence = BinOpPrecedence[value];
    if (TokenPrecedence <= 0) return std::numeric_limits<int>::max();
    return TokenPrecedence;
}
