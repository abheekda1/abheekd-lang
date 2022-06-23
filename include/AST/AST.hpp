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
void SaveModuleToFile(const std::string& path);

class Type {
public:
    inline Type(std::string Name, bool IsPointer) : Name(Name), IsPointer(IsPointer) {}

    llvm::Type *GetLLVMType(llvm::LLVMContext &Ctx) const {
        if (Name == "s1") {
            if (IsPointer) return llvm::Type::getInt8PtrTy(Ctx);
            return llvm::Type::getInt8Ty(Ctx);
        } else if (Name == "s2") {
            if (IsPointer) return llvm::Type::getInt16PtrTy(Ctx);
            return llvm::Type::getInt16Ty(Ctx);
        } else if (Name == "s4") {
            if (IsPointer) return llvm::Type::getInt32PtrTy(Ctx);
            return llvm::Type::getInt32Ty(Ctx);
        } else if (Name == "s8") {
            if (IsPointer) return llvm::Type::getInt64PtrTy(Ctx);
            return llvm::Type::getInt64Ty(Ctx);
        } else if (Name == "f4") {
            if (IsPointer) return llvm::Type::getFloatPtrTy(Ctx);
            return llvm::Type::getFloatTy(Ctx);
        } else if (Name == "f8") {
            if (IsPointer) return llvm::Type::getDoublePtrTy(Ctx);
            return llvm::Type::getDoubleTy(Ctx);
        } else if (Name == "void") {
            if (IsPointer) return nullptr;
            return llvm::Type::getVoidTy(Ctx);
        } else {
            return nullptr;
        }
    }

private:
    std::string Name;
    bool IsPointer;
};

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
    explicit NumberExprAST(std::string Value);
    llvm::Value *codegen() override;

private:
    std::string Value;
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
    PrototypeAST(std::string Name, std::vector<std::pair<std::string /* name */, Type /* type */>> Args, Type ReturnType);
    llvm::Function *codegen();

    inline const std::string getName() const { return Name; }

private:
    std::string Name;
    std::vector<std::pair<std::string, Type>> Args;
    Type ReturnType;
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
