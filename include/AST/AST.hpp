//
// Created by abheekd on 6/21/2022.
//

#ifndef ABHEEK_LANG_AST_HPP
#define ABHEEK_LANG_AST_HPP

#include <string>
#include <memory>
#include <vector>

#include "Token/Token.hpp"

//----------------------------------------------------------
// EXPRESSIONS
//----------------------------------------------------------

class ExprAST {
public:
    virtual ~ExprAST();
};

// numeric literal expressions
class NumberExprAST : public ExprAST {
public:
    explicit NumberExprAST(double Value);

private:
    double Value{};
};

// string literal expressions
class StringExprAST : public ExprAST {
public:
    explicit StringExprAST(std::string Value);

private:
    std::string Value;
};

class VariableExprAST : public ExprAST {
public:
    explicit VariableExprAST(std::string Name);

private:
    std::string Name;
};

class BinaryExprAST : public ExprAST {
public:
    explicit BinaryExprAST(Token  Op, std::unique_ptr<ExprAST> Left, std::unique_ptr<ExprAST> Right);

private:
    Token Op;
    std::unique_ptr<ExprAST> Left, Right;
};

class CallExprAST : public ExprAST {
public:
    CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args);

private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
};

class PrototypeAST {
public:
    PrototypeAST(std::string Name, std::vector<std::string> Args);

    const std::string &getName() const { return Name; }

private:
    std::string Name;
    std::vector<std::string> Args;
};

// function definition
class FunctionAST {
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::unique_ptr<ExprAST> Body);

private:
    std::unique_ptr<PrototypeAST> Proto;
    // move to block expression ast at some point
    std::unique_ptr<ExprAST> Body;
};

//----------------------------------------------------------
// STATEMENTS
//----------------------------------------------------------

class StatementAST {
public:
    ~StatementAST();
};



#endif //ABHEEK_LANG_AST_HPP
