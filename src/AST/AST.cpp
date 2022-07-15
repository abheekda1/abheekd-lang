//
// Created by abheekd on 6/21/2022.
//

#include "AST/AST.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include <iostream>

#include <utility>
#include <llvm/IR/Verifier.h>

using llvm::APFloat;
using llvm::APInt;
using llvm::BasicBlock;
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::Function;
using llvm::FunctionType;
using llvm::IRBuilder;
using llvm::LLVMContext;
using llvm::Module;
using llvm::Value;
using llvm::TargetRegistry;

// CODEGEN BEGIN
static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<IRBuilder<>> Builder;
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value *> CurrentFuncNamedValues;
static std::map<std::string, Value *> GlobalNamedValues;

int InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("holy jit", *TheContext);

  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  std::cout << "found target triple: " << TargetTriple << '\n';

  TheModule->setTargetTriple(TargetTriple);

  // Create a new builder for the module.
  Builder = std::make_unique<IRBuilder<>>(*TheContext);

  return 0;
}
// CODEGEN END

void SaveModuleToFile(const std::string &path) {
  std::error_code EC;
  llvm::raw_fd_ostream out(path, EC);
  TheModule->print(out, nullptr);
}

int SaveObjectToFile(const std::string &path) {
  std::error_code EC;
  llvm::raw_fd_ostream out(path, EC, llvm::sys::fs::OF_None);
  llvm::legacy::PassManager pass;
  auto FileType = llvm::CGFT_ObjectFile;

  using namespace llvm;
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  std::string Error;
  auto Target = TargetRegistry::lookupTarget(TheModule->getTargetTriple(), Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    errs() << Error;
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";

  TargetOptions opt;
  auto RM = Optional<Reloc::Model>();
  auto TargetMachine = Target->createTargetMachine(TheModule->getTargetTriple(), CPU, Features, opt, RM);
  TheModule->setDataLayout(TargetMachine->createDataLayout());

  if (TargetMachine->addPassesToEmitFile(pass, out, nullptr, FileType)) {
    errs() << "TargetMachine can't emit a file of this type";
    return 1;
  }

  pass.run(*TheModule);
  out.flush();
  return 0;
}

ExprAST::~ExprAST() = default;

NumberExprAST::NumberExprAST(std::string Value) : Value(std::move(Value)) {}
Value *NumberExprAST::codegen() {
  if (this->Value.find('.') != std::string::npos)
    return ConstantFP::get(*TheContext, APFloat(stod(Value)));
  // todo: contextual casting so it's not always floats
  return ConstantInt::get(*TheContext,
                          APInt(sizeof(long long) * 8, stoll(Value)));
}

StringExprAST::StringExprAST(std::string Value) : Value(std::move(Value)) {}

llvm::Value *StringExprAST::codegen() {
  return Builder->CreateGlobalStringPtr(llvm::StringRef(this->Value),
                                        "string_lit", 0, TheModule.get());
}

/*Value *StringExprAST::codegen() {
    return StringLiteral::get(TheContext, APFloat(Value));
}*/

VariableExprAST::VariableExprAST(std::string Name) : Name(std::move(Name)) {}
Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  Value *V = CurrentFuncNamedValues[Name];
  if (!V)
    throw std::runtime_error("codegen error: unknown variable name");
  return V;
}

BinaryExprAST::BinaryExprAST(Token Op, std::unique_ptr<ExprAST> Left,
                             std::unique_ptr<ExprAST> Right)
    : Op(std::move(Op)), Left(std::move(Left)), Right(std::move(Right)) {}
Value *BinaryExprAST::codegen() {
  Value *L = Left->codegen();
  Value *R = Right->codegen();
  if (!L || !R)
    return nullptr;

  // casting literals
  if (!L->hasName() != !R->hasName()) { // one or the other is constant
    // if they aren't the same let llvm deal with the different types and error
    if ((L->getType()->isIntegerTy() && R->getType()->isIntegerTy()) ||
        (L->getType()->isFloatingPointTy() &&
         R->getType()->isFloatingPointTy())) {
      if (!L->hasName()) {
        if (L->getType()->isIntegerTy())
          L = Builder->CreateIntCast(L, R->getType(), true);
        else if (L->getType()->isFloatingPointTy())
          L = Builder->CreateFPCast(L, R->getType());
      } else {
        if (R->getType()->isIntegerTy())
          R = Builder->CreateIntCast(R, L->getType(), true);
        else if (R->getType()->isFloatingPointTy())
          R = Builder->CreateFPCast(R, L->getType());
      }
    }
  }

  // todo: string ops; done
  // at this point they should be the same type
  // so L type is same as R and checking one will give
  // both
  if (Op.value == "+") {
    if (L->getType()->isIntegerTy())
      return Builder->CreateAdd(L, R, "add_tmp");
    else
      return Builder->CreateFAdd(L, R, "add_tmp");
  } else if (Op.value == "-")
    if (L->getType()->isIntegerTy())
      return Builder->CreateSub(L, R, "sub_tmp");
    else
      return Builder->CreateFSub(L, R, "sub_tmp");
  else if (Op.value == "*")
    if (L->getType()->isIntegerTy())
      return Builder->CreateMul(L, R, "mul_tmp");
    else
      return Builder->CreateFMul(L, R, "mul_tmp");
  else if (Op.value == "/")
    if (L->getType()->isIntegerTy())
      return Builder->CreateSDiv(L, R, "div_tmp");
    else
      return Builder->CreateFDiv(L, R, "div_tmp");
  else if (Op.value == "<")
    if (L->getType()->isIntegerTy())
      return Builder->CreateICmpSLT(L, R, "lt_tmp");
    else
      return Builder->CreateFCmpULT(L, R, "lt_tmp");
  else if (Op.value == ">")
    if (L->getType()->isIntegerTy())
      return Builder->CreateICmpSGT(L, R, "gt_tmp");
    else
      return Builder->CreateFCmpUGT(L, R, "gt_tmp");
  else
    throw std::runtime_error("codegen error: unknown operator");
}

CallExprAST::CallExprAST(std::string Callee,
                         std::vector<std::unique_ptr<ExprAST>> Args)
    : Callee(std::move(Callee)), Args(std::move(Args)) {}

Value *CallExprAST::codegen() {
  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF) {
    throw std::runtime_error("unknown function: \"" + Callee + "\"");
  }

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size() && !CalleeF->isVarArg())
    throw std::runtime_error("incorrect # arguments passed: expected " +
                             std::to_string(Args.size()) + ", got " +
                             std::to_string(CalleeF->arg_size()));

  std::vector<Value *> ArgsV;
  for (auto &Arg : Args) {
    ArgsV.push_back(Arg->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "call_tmp");
}

PrototypeAST::PrototypeAST(
    std::string Name,
    std::vector<std::pair<std::string /* name */, Type /* type */>> Args,
    Type ReturnType, bool IsVarArg)
    : Name(std::move(Name)), Args(std::move(Args)),
      ReturnType(std::move(ReturnType)), IsVarArg(IsVarArg) {}

llvm::Function *PrototypeAST::codegen() {
  // todo: specify types for args
  std::vector<llvm::Type *> ArgTypes(Args.size());
  for (int i = 0; i < ArgTypes.size(); i++) {
    if (auto ArgType = Args.at(i).second.GetLLVMType(*TheContext))
      ArgTypes.at(i) = ArgType;
    else
      return nullptr;
  }
  auto RetType = this->ReturnType.GetLLVMType(*TheContext);
  if (!RetType)
    throw std::runtime_error("codegen error: invalid return type");
  FunctionType *FuncType = FunctionType::get(RetType, ArgTypes, this->IsVarArg);
  Function *F = Function::Create(FuncType, Function::ExternalLinkage, Name,
                                 TheModule.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(Args[Idx++].first);
  }

  return F;
}

FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                         std::unique_ptr<StatementAST> Body)
    : Proto(std::move(Proto)), Body(std::move(Body)) {}

llvm::Function *FunctionAST::codegen() {
  // First, check for an existing function from a previous 'extern' declaration.
  Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return nullptr;

  if (!TheFunction->empty())
    throw std::runtime_error("codegen error: cannot redefine function");

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  CurrentFuncNamedValues.clear();
  for (auto &Arg : TheFunction->args())
    CurrentFuncNamedValues[Arg.getName().str()] = &Arg;

  Value *RetVal = Body->codegen();
  if (true) {
    if (TheFunction->getReturnType()->isIntegerTy()) {
      if (RetVal->getType()->isFloatingPointTy())
        RetVal = Builder->CreateFPToSI(RetVal, TheFunction->getReturnType());
      else if (RetVal->getType()->isIntegerTy())
        RetVal =
            Builder->CreateIntCast(RetVal, TheFunction->getReturnType(), true);
    } else if (TheFunction->getReturnType()->isFloatingPointTy()) {
      if (RetVal->getType()->isIntegerTy())
        RetVal = Builder->CreateSIToFP(RetVal, TheFunction->getReturnType());
      else if (RetVal->getType()->isFloatingPointTy())
        RetVal = Builder->CreateFPCast(RetVal, TheFunction->getReturnType());
    }
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    if(llvm::verifyFunction(*TheFunction)) {
        return nullptr;
    }

    return TheFunction;
  }

  TheFunction->eraseFromParent();
  return nullptr;
}

StatementAST::~StatementAST() = default;

ExprStatementAST::ExprStatementAST(std::unique_ptr<ExprAST> Expr)
    : Expr(std::move(Expr)) {
  Type = "ExprStatement";
}

llvm::Value *ExprStatementAST::codegen() { return this->Expr->codegen(); }

BlockStatementAST::BlockStatementAST(
    std::vector<std::unique_ptr<StatementAST>> Statements)
    : Statements(std::move(Statements)) {
  Type = "BlockStatement";
}

llvm::Value *BlockStatementAST::codegen() {
  for (const auto &S : Statements) {
    if (auto C = S->codegen()) {
      if (S->Type == "ReturnStatement") {
        return C;
      }
    } else {
      // return nullptr;
      throw std::runtime_error("codegen error: failed to compile function body");
    }
  }
  return nullptr;
}

ReturnStatementAST::ReturnStatementAST(std::unique_ptr<ExprAST> Argument)
    : Argument(std::move(Argument)) {
  Type = "ReturnStatement";
}

llvm::Value *ReturnStatementAST::codegen() { return this->Argument->codegen(); }

VarDeclStatementAST::VarDeclStatementAST(std::unique_ptr<ExprAST> Var,
                                         std::string Type)
    : Var(std::move(Var)), Type(std::move(Type)) {}

// todo
llvm::Value *VarDeclStatementAST::codegen() { return nullptr; }
