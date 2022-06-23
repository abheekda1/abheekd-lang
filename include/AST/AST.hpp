//
// Created by abheekd on 6/21/2022.
//

#ifndef ABHEEK_LANG_AST_HPP
#define ABHEEK_LANG_AST_HPP

#include <string>
#include <memory>
#include <vector>

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

#include "Token/Token.hpp"

void InitializeModule();

//----------------------------------------------------------
// EXPRESSIONS
//----------------------------------------------------------

class ExprAST {
public:
    virtual ~ExprAST();
    virtual llvm::Value *codegen() = 0;
};

// numeric literal expressions
class NumberExprAST : public ExprAST {
public:
    explicit NumberExprAST(double Value);
    llvm::Value *codegen() override;

private:
    double Value{};
};

// string literal expressions
class StringExprAST : public ExprAST {
public:
    explicit StringExprAST(std::string Value);
    llvm::Value *codegen() override;

private:
    std::string Value;
};

class VariableExprAST : public ExprAST {
public:
    explicit VariableExprAST(std::string Name);


    llvm::Value *codegen() override;

private:
    std::string Name;
};

class BinaryExprAST : public ExprAST {
public:
    explicit BinaryExprAST(Token  Op, std::unique_ptr<ExprAST> Left, std::unique_ptr<ExprAST> Right);
    llvm::Value *codegen() override;

private:
    Token Op;
    std::unique_ptr<ExprAST> Left, Right;
};

class CallExprAST : public ExprAST {
public:
    CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args);
    llvm::Value *codegen() override;

private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
};

//----------------------------------------------------------
// STATEMENTS
//----------------------------------------------------------

class StatementAST {
public:
    virtual ~StatementAST();
    virtual llvm::Value *codegen() = 0;

    // todo: enum
    std::string Type;
};

class ExprStatementAST : public StatementAST {
public:
    explicit ExprStatementAST(std::unique_ptr<ExprAST> Expr);
    llvm::Value *codegen() override;

private:
    std::unique_ptr<ExprAST> Expr;
};

class BlockStatementAST : public StatementAST {
public:
    explicit BlockStatementAST(std::vector<std::unique_ptr<StatementAST>> Statements);
    llvm::Value *codegen() override;

    inline const std::vector<std::unique_ptr<StatementAST>> &getStatements() const { return Statements; }

private:
    std::vector<std::unique_ptr<StatementAST>> Statements;
};

class ReturnStatementAST : public StatementAST {
public:
    explicit ReturnStatementAST(std::unique_ptr<ExprAST> Argument);
    llvm::Value *codegen() override;

private:
    std::unique_ptr<ExprAST> Argument;
};

class VarDeclStatementAST : public StatementAST {
public:
    // todo: enum type instead of string
    explicit VarDeclStatementAST(std::unique_ptr<ExprAST> Var, std::string Type);
    llvm::Value *codegen() override;

private:
    std::unique_ptr<ExprAST> Var;
    std::string Type;
};



class PrototypeAST {
public:
    PrototypeAST(std::string Name, std::vector<std::string> Args, std::string ReturnType);
    llvm::Function *codegen();

    inline const std::string getName() const { return Name; }
    inline llvm::Type *getReturnType(llvm::LLVMContext &Ctx) const {
        if (ReturnType == "s1") {
            return llvm::Type::getInt8Ty(Ctx);
        } else if (ReturnType == "s2") {
            return llvm::Type::getInt16Ty(Ctx);
        } else if (ReturnType == "s4") {
            return llvm::Type::getInt32Ty(Ctx);
        } else if (ReturnType == "s8") {
            return llvm::Type::getInt64Ty(Ctx);
        } else if (ReturnType == "f4") {
            return llvm::Type::getFloatTy(Ctx);
        } else if (ReturnType == "f8") {
            return llvm::Type::getDoubleTy(Ctx);
        } else {
            return nullptr;
        }
    }
    inline const std::string getReturnTypeStr() const { return ReturnType; }

private:
    std::string Name;
    std::vector<std::string> Args;
    std::string ReturnType;
};

// function definition
class FunctionAST {
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::unique_ptr<StatementAST> Body);
    llvm::Function *codegen();

private:
    std::unique_ptr<PrototypeAST> Proto;
    // move to block expression ast at some point; update: should be done
    std::unique_ptr<StatementAST> Body;
};

#endif //ABHEEK_LANG_AST_HPP
