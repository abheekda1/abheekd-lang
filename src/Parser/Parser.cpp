//
// Created by abheekd on 6/21/2022.
//

#include <limits>

#include "Parser/Parser.hpp"

std::map<char, int> Parser::BinOpPrecedence;
Token Parser::CurrentToken;

Parser::Parser() {
    BinOpPrecedence['*'] = 5;
    BinOpPrecedence['/'] = 5;
    BinOpPrecedence['%'] = 5;

    BinOpPrecedence['+'] = 6;
    BinOpPrecedence['-'] = 6;
}

std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
    auto ret = std::make_unique<NumberExprAST>(std::stod(CurrentToken.value));
    getNextToken(); // eat literal
    return ret;
}

std::unique_ptr<ExprAST> Parser::ParseStringExpr() {
    auto ret = std::make_unique<StringExprAST>(CurrentToken.value);
    getNextToken(); // eat literal
    return ret;
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    getNextToken(); // eat (
    auto V = ParseExpression();
    if (!V)
        return nullptr;

    if (CurrentToken.value != ")")
        throw std::runtime_error("parser error: expected ')'");
    getNextToken(); // eat )
    return V;
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
    std::string IdName = CurrentToken.value;
    getNextToken(); // eat ident

    if (CurrentToken.value != "(") // if it's just a variable and not a call
        return std::make_unique<VariableExprAST>(IdName);

    // function call
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurrentToken.value != ")") {
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurrentToken.value == ")")
                break;

            if (CurrentToken.value != ",")
                throw std::runtime_error("parser error: expected only ')', ',', or expression in arg list");

            getNextToken();
        }
    }

    getNextToken(); // eat ')'

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
    switch  (CurrentToken.type) {
        case Token::type::tok_ident:
            return ParseIdentifierExpr();
        case Token::type::tok_number:
            return ParseNumberExpr();
        case Token::type::tok_string:
            return ParseStringExpr();
        case Token::type::tok_other:
            if (CurrentToken.value == "(")
                return ParseParenExpr();
        default:
            throw std::runtime_error("unknown token '" + CurrentToken.value + "'");
    }
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    auto Left = ParsePrimary();
    if (!Left) return nullptr;

    return ParseBinOpRight(std::numeric_limits<int>::max() - 1, std::move(Left));
}

int Parser::GetTokenPrecedence() {
    if (!isascii(CurrentToken.value.at(0)))
        return std::numeric_limits<int>::max();

    // make sure it has been declared
    int TokenPrecedence = BinOpPrecedence[CurrentToken.value.at(0)];
    if (TokenPrecedence <= 0) return std::numeric_limits<int>::max();
    return TokenPrecedence;
}

std::unique_ptr<ExprAST> Parser::ParseBinOpRight(int ExprPrecedence, std::unique_ptr<ExprAST> Left) {
    while (true) {
        int TokenPrecedence = GetTokenPrecedence();

        if (TokenPrecedence > ExprPrecedence)
            return Left;

        Token BinaryOp = CurrentToken;
        getNextToken(); // eat binop

        // parse the primary expr after the operator
        auto Right = ParsePrimary();
        if (!Right)
            return nullptr;

        // determine association
        int NextPrecedence = GetTokenPrecedence();
        if (TokenPrecedence > NextPrecedence) {
            Right = ParseBinOpRight(TokenPrecedence - 1, std::move(Right));
            if (!Right)
                return nullptr;
        }
        // merge both sides
        Left = std::make_unique<BinaryExprAST>(BinaryOp, std::move(Left), std::move(Right));
    }
}

std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
    if (CurrentToken.type != Token::type::tok_ident)
        throw std::runtime_error("parser error: expected function name in prototype");

    std::string Name = CurrentToken.value;
    getNextToken(); // eat name

    if (CurrentToken.value != "(")
        throw std::runtime_error("parser error: expected '(' in prototype");

    // read args
    std::vector<std::string> ArgNames;
    while (getNextToken().type == Token::type::tok_ident)
        ArgNames.push_back(CurrentToken.value);
    if (CurrentToken.value != ")")
        throw std::runtime_error("parser error: missing terminating ')' in prototype");

    getNextToken(); // eat ')'

    return std::make_unique<PrototypeAST>(Name, std::move(ArgNames));
}

std::unique_ptr<FunctionAST> Parser::ParseFuncDefinition() {
    getNextToken(); // eat func keyword
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;

    // TODO: add block expression
    if (auto E = ParseStatement())
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
    getNextToken(); // eat extern
    return ParsePrototype();
}

std::unique_ptr<StatementAST> Parser::ParseStatement() {
    switch (CurrentToken.type) {
        case Token::type::tok_return:
            return ParseReturnStatement();
        default:
            if (CurrentToken.value == "{") {
                return ParseBlockStatement();
            }
            return ParseExprStatement();
    }
}

std::unique_ptr<StatementAST> Parser::ParseExprStatement() {
    if (auto E = ParseExpression()) {
        if (CurrentToken.value != ";") {
            throw std::runtime_error("parser error: missing semicolon at the end of statement");
        }
        getNextToken(); // eat ';'
        return std::make_unique<ExprStatementAST>(std::move(E));
    }
    return nullptr;
}

std::unique_ptr<StatementAST> Parser::ParseBlockStatement() {
    getNextToken(); // eat {
    std::vector<std::unique_ptr<StatementAST>> Statements;
    while (CurrentToken.value != "}") {
        if (auto V = ParseStatement())
            Statements.push_back(std::move(V));
        else
            return nullptr;
    }

    getNextToken(); // eat }
    return std::make_unique<BlockStatementAST>(std::move(Statements));
}

std::unique_ptr<StatementAST> Parser::ParseReturnStatement() {
    getNextToken(); // eat "return"

    if (auto Arg = ParseExpression()) {
        if (CurrentToken.value != ";") {
            throw std::runtime_error("parser error: missing semicolon at the end of return statement");
        }
        getNextToken(); // eat ';'
        return std::make_unique<ReturnStatementAST>(std::move(Arg));
    }
    return nullptr;
}
