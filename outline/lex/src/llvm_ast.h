// llvm_ast.h
#ifndef LLVM_AST_H
#define LLVM_AST_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

// Forward-declare LLVM globals (they will be defined in llvm_compiler.cpp)
extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;
extern llvm::Module TheModule;

// Global symbol table mapping variable names to their alloca instructions.
extern std::unordered_map<std::string, llvm::Value *> NamedValues;

// Base AST Node class.
class ASTNode {
public:
  virtual ~ASTNode() {}
  virtual llvm::Value *codegen() = 0;
};

// Variable Declaration AST Node.
class VarDeclNode : public ASTNode {
public:
  std::string type;
  std::string name;
  ASTNode *value; // initializer (can be nullptr)

  VarDeclNode(const std::string &type, const std::string &name, ASTNode *value)
      : type(type), name(name), value(value) {}

  llvm::Value *codegen() override;
};

// Number AST Node.
class NumberNode : public ASTNode {
public:
  std::string value;
  std::string inferredType; // "int" or "float"

  NumberNode(const std::string &value, const std::string &inferredType)
      : value(value), inferredType(inferredType) {}

  llvm::Value *codegen() override;
};

// Assignment AST Node.
class AssignmentNode : public ASTNode {
public:
  std::string name;
  ASTNode *value;

  AssignmentNode(const std::string &name, ASTNode *value)
      : name(name), value(value) {}

  llvm::Value *codegen() override;
};

// Variable Reference Node (for use in expressions).
class VariableNode : public ASTNode {
public:
  std::string name;
  VariableNode(const std::string &name) : name(name) {}
  llvm::Value *codegen() override;
};

// Print Statement Node.
class PrintNode : public ASTNode {
public:
  ASTNode *expr; // Expression to print.
  PrintNode(ASTNode *expr) : expr(expr) {}
  llvm::Value *codegen() override;
};

#endif // LLVM_AST_H