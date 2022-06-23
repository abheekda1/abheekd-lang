//
// Created by abheekd on 6/21/2022.
//

#ifndef ABHEEK_LANG_TOKEN_HPP
#define ABHEEK_LANG_TOKEN_HPP

#include <map>
#include <string>

struct position {
    int row = 1; // line
    int column = 0;
};

class Token {
public:
    enum type {
        tok_eof = -1,

        // commands
        tok_func = -2,
        tok_extern = -3,
        tok_return = -4,
        tok_var = -5,

        // primary
        tok_ident = -20,
        tok_number = -21,
        tok_string = -22,

        // operators
        tok_binop = -40,

        //other
        tok_other = -100
    };

    Token();
    explicit Token(type type);
    Token(type type, std::string value, position position);

    static std::map<std::string, int> BinOpPrecedence;
    static void InitBinOps();

    int GetPrecedence();
    inline bool IsBinOp() const { return BinOpPrecedence[value]; }

    type type;
    std::string value;
    position position;
};


#endif //ABHEEK_LANG_TOKEN_HPP
