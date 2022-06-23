//
// Created by abheekd on 6/21/2022.
//

#ifndef ABHEEK_LANG_PARSER_HPP
#define ABHEEK_LANG_PARSER_HPP

#include <map>

#include "AST/AST.hpp"
#include "Lexer/Lexer.hpp"
#include "Token/Token.hpp"

class Parser {
public:
    // ctor
    // Parser();

    static inline Token getNextToken() { return CurrentToken = Lexer::getTok(); }
    static Token CurrentToken;

    // EXPRESSION BEGIN
    static std::unique_ptr<ExprAST> ParseExpression();

    static std::unique_ptr<ExprAST> ParseNumberExpr();
    static std::unique_ptr<ExprAST> ParseStringExpr();
    static std::unique_ptr<ExprAST> ParseParenExpr();
    static std::unique_ptr<ExprAST> ParseIdentifierExpr();

    static std::unique_ptr<ExprAST> ParsePrimary();

    static std::unique_ptr<PrototypeAST> ParsePrototype();
    static std::unique_ptr<FunctionAST> ParseFuncDefinition();
    static std::unique_ptr<PrototypeAST> ParseExtern();

    static std::unique_ptr<ExprAST> ParseBinOpRight(int Precedence, std::unique_ptr<ExprAST> Left);
    // EXPRESSION END

    // STATEMENT BEGIN
    static std::unique_ptr<StatementAST> ParseStatement();

    static std::unique_ptr<StatementAST> ParseExprStatement();
    static std::unique_ptr<StatementAST> ParseBlockStatement();
    static std::unique_ptr<StatementAST> ParseReturnStatement();
    static std::unique_ptr<StatementAST> ParseVarDeclStatement();
    //STATEMENT END
};


#endif //ABHEEK_LANG_PARSER_HPP
