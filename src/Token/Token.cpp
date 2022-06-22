//
// Created by abheekd on 6/21/2022.
//

#include <string>
#include <utility>

#include "Token/Token.hpp"

Token::Token() : type(type::tok_other) {}
Token::Token(enum type type) : type(type) {}
Token::Token(enum type type, std::string value, struct position position) :
        type(type), value(std::move(value)), position(position) {}