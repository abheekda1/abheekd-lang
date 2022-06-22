//
// Created by abheekd on 6/21/2022.
//

#include "AST/AST.hpp"

#include <utility>

ExprAST::~ExprAST() = default;

NumberExprAST::NumberExprAST(double Value) : Value(Value) {}

StringExprAST::StringExprAST(std::string Value) : Value(std::move(Value)) {}

VariableExprAST::VariableExprAST(std::string Name) : Name(std::move(Name)) {}

BinaryExprAST::BinaryExprAST(Token  Op, std::unique_ptr<ExprAST> Left,
                             std::unique_ptr<ExprAST> Right)
                             : Op(std::move(Op)), Left(std::move(Left)),
                             Right(std::move(Right)) {}

CallExprAST::CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args)
                         : Callee(std::move(Callee)), Args(std::move(Args)) {}

PrototypeAST::PrototypeAST(std::string Name, std::vector<std::string> Args)
                           : Name(std::move(Name)), Args(std::move(Args)) {}

FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<StatementAST> Body)
                         : Proto(std::move(Proto)), Body(std::move(Body)) {}


StatementAST::~StatementAST() = default;

ExprStatementAST::ExprStatementAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}

BlockStatementAST::BlockStatementAST(std::vector<std::unique_ptr<StatementAST>> Statements)
                                     : Statements(std::move(Statements)) {}
