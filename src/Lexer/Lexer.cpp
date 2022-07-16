//
// Created by abheekd on 6/21/2022.
//

#include "Lexer/Lexer.hpp"

#include <utility>
#include <stdexcept>
#include "Token/Token.hpp"

// static members
char Lexer::LastChar = ' ';
int Lexer::CharIdx = 0;
position Lexer::Position;
std::string Lexer::Source;

Token Lexer::getTok() {
    LastChar = Source[CharIdx];

    if (isspace(LastChar)) {
        while (isspace(LastChar)) {
            switch (LastChar) {
                case '\n':
                    Position.row++;
                    Position.column = 0;
                    break;
                case ' ':
                case '\t':
                    Position.column++;
                    break;
                default:
                    break;
            }
            LastChar = Source[++CharIdx];
        }
    }

    if (isalpha(LastChar)) {
        Token t(Token::type::tok_ident);
        while (isalnum((LastChar = Source[CharIdx++]))) {
            t.value += LastChar;
        }

        CharIdx--; // go back one since one was added after the while loop

        // deal with keywords
        if (t.value == "func") t.type = Token::type::tok_func;
        if (t.value == "extern") t.type = Token::type::tok_extern;
        if (t.value == "return") t.type = Token::type::tok_return;
        if (t.value == "var") t.type = Token::type::tok_var;

        t.pos = Position;
        Position.column += (int)t.value.length();
        return t;
    }

    if (isdigit(LastChar)/* || LastChar == '.'*/) {
        bool foundPoint = false;

        Token t(Token::type::tok_number);
        t.pos = Position;
        while (isdigit((LastChar = Source[CharIdx++])) || LastChar /* already set by prev cond */ == '.') {
            if (foundPoint) {
                if (LastChar == '.') { // multiple decimal points
                    // TODO: implement error interface
                    throw std::runtime_error("lexer error: found extra point in number");
                }
            } else {
                foundPoint = (LastChar == '.');
            }

            t.value += LastChar;
        }

        CharIdx--; // go back one since one was added after the while loop

        Position.column += (int)t.value.length();
        return t;
    }

    if (LastChar == '"') {
        Token t(Token::type::tok_string);
        t.pos = Position;

        Position.column++; // preemptively increment for first quotation mark
        while((LastChar = Source[++CharIdx]) != '"' /* second mark */) { // move past first quotation mark
            if (LastChar == '\n' || CharIdx == Source.length())
                throw std::runtime_error("lexer error: unterminated string");
            // TODO: handle escape codes: https://en.cppreference.com/w/cpp/language/escape
            if (LastChar == '\\') {
                LastChar = Source[++CharIdx];
                Position.column++;
                switch(LastChar) {
                    case '\'':
                        t.value += '\x27';
                        break;
                    case '"':
                        t.value += '\x22';
                        break;
                    case '?':
                        t.value += '\x3f';
                        break;
                    case '\\':
                        t.value += '\x5c';
                        break;
                    case 'a':
                        t.value += '\x7';
                        break;
                    case 'b':
                        t.value += '\x8';
                        break;
                    case 'f':
                        t.value += '\xc';
                        break;
                    case 'n':
                        t.value += '\x0a';
                        break;
                    case 'r':
                        t.value += '\x0d';
                        break;
                    case 't':
                        t.value += '\x09';
                        break;
                    case 'v':
                        t.value += '\x0b';
                        break;
                    // case 'x': // hexadecimal byte
                    //     std::string temp;
                    //     while (isdigit(LastChar = Source[++CharIdx])) {
                    //         Position.column++;
                    //         temp += LastChar;
                    //         std::cout << temp << std::endl;
                    //     }
                    //     CharIdx--;
                    //     std::cout << '\n' << std::stoull(temp) << std::endl;
                    //     t.value += std::stoull(temp);
                }
                Position.column++;
            } else {
                t.value += LastChar;
                Position.column++;
            }
        }

        CharIdx++; // eat terminating '"'
        Position.column++;
        return t;
    }

    if (Token::BinOpPrecedence[{LastChar}]) { // check if first char is op
        Token t(Token::type::tok_binop);
        t.value = {LastChar};
        t.pos = Position;

        CharIdx++; // move past first char

        while (Token::BinOpPrecedence[{(LastChar = Source[CharIdx++])}]) {
            t.value += LastChar;
        }

        CharIdx--; // go back one since one was added after the while loop

        Position.column += (int)t.value.length();
        return t;
    }

    if (CharIdx == Source.length())
        return Token{Token::type::tok_eof, std::string(), Position};

    Token otherTok = Token{Token::type::tok_other, std::string({Source[CharIdx]}), Position};
    CharIdx++;
    Position.column++;
    return otherTok;
}

Lexer::Lexer() = default;
Lexer::Lexer(std::string source) {
    Source = std::move(source);
};

Lexer::~Lexer() = default;
