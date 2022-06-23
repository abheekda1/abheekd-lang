//
// Created by abheekd on 6/21/2022.
//

#include <limits>
#include <string>

#include "Parser/Parser.hpp"

Token Parser::CurrentToken;

std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
    auto ret = std::make_unique<NumberExprAST>(CurrentToken.value);
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
    // todo: maybe `while CurrentToken.value != ")"...`
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

std::unique_ptr<ExprAST> Parser::ParseBinOpRight(int ExprPrecedence, std::unique_ptr<ExprAST> Left) {
    while (true) {
        int TokenPrecedence = CurrentToken.GetPrecedence();

        if (TokenPrecedence > ExprPrecedence)
            return Left;

        Token BinaryOp = CurrentToken;
        getNextToken(); // eat binop

        // parse the primary expr after the operator
        auto Right = ParsePrimary();
        if (!Right)
            return nullptr;

        // determine association
        int NextPrecedence = CurrentToken.GetPrecedence();
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

    getNextToken(); // eat '('
    // read args
    std::vector<std::pair<std::string /* name */, Type /* type */>> Args;
    if (CurrentToken.value != ")") {
        while (true) { // loop through each arg
            std::string ArgName;
            std::string ArgTypeName;
            bool ArgTypePointer = false;

            if (CurrentToken.type == Token::type::tok_ident)
                ArgName = CurrentToken.value;
            else
                return nullptr;

            getNextToken(); // eat arg name

            if (CurrentToken.value != ":")
                throw std::runtime_error("parser error: expected ':' between arg name and type");
            getNextToken(); // eat ':'

            if (CurrentToken.type == Token::type::tok_ident)
                ArgTypeName = CurrentToken.value;
            else
                return nullptr;

            if (getNextToken().value == "*") { // eat type and check for pointer
                ArgTypePointer = true;
                getNextToken(); // eat '*'
            }

            Args.emplace_back(ArgName, Type(ArgTypeName, ArgTypePointer));

            if (CurrentToken.value == ")")
                break;

            if (CurrentToken.value != ",")
                throw std::runtime_error("parser error: expected only ')', ',', or expression in arg list");

            getNextToken();
        }
    }

    getNextToken(); // eat ')'

    if (CurrentToken.value != ":")
        throw std::runtime_error("parser error: expected ':' before return type");
    getNextToken(); // eat ':'
    std::string RetTypeName = CurrentToken.value;
    bool RetTypePointer = false;
    if (getNextToken().value == "*") { // eat type and check for pointer
        RetTypePointer = true;
        getNextToken(); // eat '*'
    }

    return std::make_unique<PrototypeAST>(Name, std::move(Args), Type(RetTypeName, RetTypePointer));
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
        case Token::type::tok_var:
            return ParseVarDeclStatement();
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

std::unique_ptr<StatementAST> Parser::ParseVarDeclStatement() {
    getNextToken(); // eat "var"

    if (auto Var = ParseExpression()) {
        if (CurrentToken.value != ":") {
            throw std::runtime_error("parser error: expected ':' separating var name and type");
        }
        std::string Type = getNextToken().value; // eat ':' and get type
        getNextToken(); // eat type
        return std::make_unique<VarDeclStatementAST>(std::move(Var), std::move(Type));
    }

    return nullptr;
}
